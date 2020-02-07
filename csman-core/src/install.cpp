//
// Created by kiva on 2020/2/5.
//

#include <csman/os/os.hpp>
#include <csman/core/ops.hpp>
#include <csman/core/network.hpp>
#include <sstream>
#include <fstream>
#include <utility>
#include <json/writer.h>
#include <csman/core/parser.hpp>

namespace {
    using namespace csman::core;

    struct install_error : public std::runtime_error {
        source_package _pkg;

        install_error(source_package pkg, const std::string &reason)
            : std::runtime_error(reason),
              _pkg(std::move(pkg)) {}
    };

    bool version_satisfy(const std::string &required, const std::string &version) {
        // TODO: support wildcards in version
        return required == version;
    }

    std::string type_dir(source_content_type type) {
        switch (type) {
            case source_content_type::DLL:
                return "lib";
            case source_content_type::BIN:
                return "bin";
            case source_content_type::CSE:
            case source_content_type::CSP:
                return "imports";
            default:
                return "f**k-you";
        }
    }

    std::ios::openmode openmode_for(source_content_type type) {
        switch (type) {
            case source_content_type::DLL:
            case source_content_type::BIN:
            case source_content_type::CSE:
            case source_content_type::ZIP:
                return std::ios::out | std::ios::binary;
            default:
                return std::ios::out;
        }
    }

    struct file_downloader : public download_task<std::fstream> {
        explicit file_downloader(const std::string &url)
            : download_task<std::fstream>(url) {
            // write to file stream
            this->on("write", [](std::fstream *buffer, const char *data,
                                 size_t size, size_t nmemb, size_t *wrote) {
                buffer->write(data, size * nmemb);
                *wrote = size * nmemb;
            });
        }

        ~file_downloader() override = default;

        bool open(const std::string &file, std::ios::openmode mode) {
            get_buffer().open(file, mode);
            return get_buffer().good();
        }
    };
}

namespace csman {
    namespace core {
        using namespace csman::os;

        void operation::checkout_version(mpp::event_emitter &ev, local_version &version) {
            // first of all, make sure that the version exists.
            // we only check the versions/<version>'s existence
            // and we don't care what's actually inside.
            if (!OS::current()->directory_exists(version._path)) {
                ev.emit("co-error", std::string(
                    "version " + version._name + " doesn't exists: "
                    + OS::current()->error()));
                return;
            }

            // secondly, create symbolic link
            // from versions/current to versions/<version>
            std::string current_dir =
                _core->_version_dir._path + mpp::path_separator + "current";
            if (!OS::current()->ln(version._path, current_dir)) {
                ev.emit("co-error", std::string(
                    "failed to create symlink from " + version._path
                    + " to " + current_dir + ": " + OS::current()->error()));
                return;
            }

            // finally, update config
            _core->set_current_version(version._name);
            ev.emit("co-ok");
        }

        void operation::remove_version(mpp::event_emitter &ev, local_version &version) {
            // first of all, unset current version
            // if we are removing it
            ev.emit("rv-progress", 10);
            auto &&cv = optional_current_version();
            if (cv.has_value() && version._name == cv.get()) {
                std::string current_dir =
                    _core->_version_dir._path + mpp::path_separator + "current";
                ev.emit("rv-progress", 40);
                if (!OS::current()->unlink(current_dir)) {
                    ev.emit("rv-error", std::string(
                        "failed to remove current version " + version._name
                        + ": unable to remove symlink: "
                        + OS::current()->error()));
                    return;
                }
                _core->unset_current_version();
            }

            // secondly, remove everything under version.
            // DO NOT check errors here, because we have removed
            // current version config globally,
            // just proceed when error occurs.
            ev.emit("rv-progress", 70);
            OS::current()->rm_rf(version._path);

            // and... that's all
            ev.emit("rv-progress", 100);
            ev.emit("rv-ok");
        }

        void operation::install_version(mpp::event_emitter &ev, source_version &version) {
            // version package is also a package
            auto &&vpkgs = query_package(version._version._package_rtm);
            if (vpkgs.empty()) {
                ev.emit("iv-error", std::string(
                    "failed to locate version " + version._version._name
                    + ": version package " + version._version._package_rtm
                    + " not found"));
                return;
            }

            for (auto &p : vpkgs) {
                if (p._package._name == version._version._package_rtm) {
                    install_package(ev, p);
                    // no need to enumerate
                    break;
                }
            }
        }

        void operation::remove_package(mpp::event_emitter &ev, local_package &pkg) {
            // first of all, make sure that this package exists.
            std::string package_contents_root =
                _core->_version_dir._path + mpp::path_separator + pkg._owner_version;
            if (!OS::current()->directory_exists(pkg._path)) {
                ev.emit("rp-error", std::string(
                    "package " + pkg._info._name + " doesn't exists in version "
                    + pkg._owner_version + ": " + OS::current()->error()));
                return;
            }

            // secondly, remove package files
            int files = pkg._files.size();
            int deleted = 0;
            ev.emit("rp-progress", 0);
            std::stringstream ss;
            for (auto &f : pkg._files) {
                ss << package_contents_root;
                ss << mpp::path_separator;
                ss << f;
                OS::current()->rm_rf(ss.str());
                ss.str(std::string());
                ss.clear();
                ev.emit("rp-progress", static_cast<int>(++deleted / files));
            }

            // finally, remove package info dir
            OS::current()->rm_rf(pkg._path);
            ev.emit("rp-progress", 100);
            ev.emit("rp-ok");
        }

        void operation::install_package(mpp::event_emitter &ev, source_package &pkg) {
            // first of all, collect packages need to install
            ev.emit("ip-progress", 0, std::string("Collecting dependencies"));

            std::vector<source_package> deps;
            try {
                collect_deps(deps, pkg);
            } catch (source_error &err) {
                ev.emit("ip-error", std::string(err.what()));
                return;
            }

            int progress = 10;
            const int total = deps.size();
            const int progress_each = (100 - progress) / total;

            try {
                for (auto &dep : deps) {
                    ev.emit("ip-progress", progress, std::string(
                        "Installing " + dep._package._name + "(" + dep._package._version + ")"));
                    install_internal(ev, dep);
                    progress += progress_each;
                }
            } catch (const install_error &error) {
                ev.emit("ip-error", std::string(
                    "error while installing package "
                    + error._pkg._package._name + "("
                    + error._pkg._package._version
                    + "): " + error.what()));
                return;
            }

            ev.emit("ip-ok");
        }

        void operation::install_internal(mpp::event_emitter &ev, const source_package &pkg) {
            std::string version_root =
                _core->_version_dir._path + mpp::path_separator + pkg._owner_version;
            std::string packages_dir =
                version_root + mpp::path_separator + "packages";
            std::string package_root =
                packages_dir + mpp::path_separator + pkg._package._name;

            if (!OS::current()->directory_exists(version_root)
                && !OS::current()->mkdir(version_root)) {
                throw install_error(pkg,
                    "unable to mkdir " + version_root
                    + ": " + OS::current()->error());
            }

            if (!OS::current()->directory_exists(packages_dir)
                && !OS::current()->mkdir(packages_dir)) {
                throw install_error(pkg,
                    "unable to mkdir " + packages_dir
                    + ": " + OS::current()->error());
            }

            if (!OS::current()->directory_exists(package_root)
                && !OS::current()->mkdir(package_root)) {
                throw install_error(pkg,
                    "unable to mkdir " + package_root
                    + ": " + OS::current()->error());
            }

            std::string info_file = package_root + mpp::path_separator + "info.json";
            std::fstream stream(info_file, std::ios::out);
            if (!stream.good()) {
                throw install_error(pkg, "unable to create package info file");
            }

            Json::Value files;
            int i = 0;

            for (auto &content : pkg._package._contents) {
                if (content._type == source_content_type::UNKNOWN) {
                    // impossible
                    throw install_error(pkg, content._name + " has unknown content type");
                }

                if (content._type == source_content_type::ZIP) {
                    // TODO: support zip
                    throw install_error(pkg, content._name + ": zip is WIP");
                }

                // create needed directories
                std::string dir_name = type_dir(content._type);
                std::string parent_path = version_root + mpp::path_separator + dir_name;

                if (!OS::current()->directory_exists(parent_path)
                    && !OS::current()->mkdir(parent_path)) {
                    throw install_error(pkg,
                        "unable to mkdir " + parent_path
                        + ": " + OS::current()->error());
                }

                // record files
                std::string file_path = parent_path + mpp::path_separator + content._name;
                files[i++] = dir_name + mpp::path_separator + content._name;

                // download file from source
                std::string file_url = pkg._package._base_url + "/" + content._name;
                file_downloader downloader(file_url);
                downloader.open(file_path, openmode_for(content._type));
                downloader.on("progress", [&](int progress) {
                    ev.emit("ip-net-progress", progress);
                });
                downloader.on("error", [&](const std::string &reason) {
                    // this error will be thrown from downloader.perform()
                    // and return to caller
                    throw install_error(pkg, "failed to download content "
                                             + content._name + ": " + reason);
                });
                downloader.perform();
            }

            // save package info
            Json::Value root;
            root["info"] = jsonify(pkg._package);
            root["files"] = files;
            save_json_stream(stream, root);
        }

        void operation::collect_deps(std::vector<source_package> &deps, source_package &pkg) {
            std::set<std::string> names;
            collect_deps_impl(names, deps, pkg);
        }

        void operation::collect_deps_impl(std::set<std::string> &names,
                                          std::vector<source_package> &deps,
                                          source_package &pkg) {
            if (names.find(pkg._package._name) != names.end()) {
                return;
            }

            // for-all a, a depends on a
            deps.push_back(pkg);
            names.insert(pkg._package._name);

            // for-all dep belongs to a, a depends on dep
            for (auto &name_and_ver : pkg._package._deps) {
                bool found = false;
                auto &&result = query_package(name_and_ver.first);

                for (auto &&dep : result) {
                    if (dep._package._name != name_and_ver.first) {
                        continue;
                    }
                    // ok, we found the required package
                    // check version now
                    found = true;
                    if (version_satisfy(name_and_ver.second, dep._package._version)) {
                        // version satisfied, collect dep's deps
                        collect_deps_impl(names, deps, dep);
                    } else {
                        throw_ex("Dependency not satisfied on " + name_and_ver.first
                                 + "(required by " + pkg._package._name + "): "
                                 + "version not satisfied: required " + name_and_ver.second
                                 + ", but got " + dep._package._version);
                    }
                }

                if (!found) {
                    throw_ex("Dependency not satisfied on " + name_and_ver.first
                             + "(required by " + pkg._package._name + "): "
                             + "cannot locate package " + name_and_ver.first);
                }
            }

            // that's all
        }
    }
}
