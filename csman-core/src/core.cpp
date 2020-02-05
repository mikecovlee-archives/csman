//
// Created by kiva on 2020/1/31.
//

#include <csman/os/os.hpp>
#include <csman/core/core.hpp>
#include <csman/core/parser.hpp>
#include <csman/core/ops.hpp>
#include <json/writer.h>
#include <fstream>

namespace {
    using namespace csman::core;

    std::string jsonify(source_content_type type) {
        switch (type) {
            case source_content_type::UNKNOWN:
                return "UNKNOWN";
            case source_content_type::DLL:
                return "DLL";
            case source_content_type::BIN:
                return "BIN";
            case source_content_type::CSE:
                return "CSE";
            case source_content_type::CSP:
                return "CSP";
            case source_content_type::ZIP:
                return "ZIP";
        }
    }

    Json::Value jsonify(source_content_info &info) {
        Json::Value node;
        node[0] = jsonify(info._type);
        node[1] = info._name;
        for (auto &meta : info._meta) {
            node[2][meta.first] = meta.second;
        }
        return std::move(node);
    }

    Json::Value jsonify(source_package_info &info) {
        Json::Value node;
        node[KEY_NAME] = info._display_name;
        node[KEY_PNAME] = info._name;
        node[KEY_PFNAME] = info._full_name;
        node[KEY_BASE_URL] = info._base_url;
        node[KEY_VERSION] = info._version;

        auto &deps = node[KEY_DEPS];
        for (auto &dep : info._deps) {
            deps[dep.first] = dep.second;
        }

        auto &contents = node[KEY_CONTENTS];
        int i = 0;
        for (auto &content : info._contents) {
            contents[i++] = jsonify(content);
        }

        return std::move(node);
    }

    Json::Value jsonify(source_version_info &info) {
        Json::Value node;
        node[KEY_NAME] = info._name;
        node[KEY_BASE_URL] = info._base_url;
        node[KEY_RTM] = info._package_rtm;
        node[KEY_DEV] = info._package_dev;

        auto &pkgs = node[KEY_PKG];
        int i = 0;
        for (auto &pkg : info._packages) {
            pkgs[i++] = jsonify(pkg.second);
        }

        return std::move(node);
    }

    Json::Value jsonify(source_platform_info &info) {
        Json::Value node;
        node[KEY_NAME] = info._name;
        node[KEY_BASE_URL] = info._base_url;
        node[KEY_VERSION] = info._version_default;
        node[KEY_LATEST] = info._version_latest;
        node[KEY_NIGHTLY] = info._version_nightly;

        auto &versions = node[KEY_VERSIONS];
        int i = 0;
        for (auto &version : info._versions) {
            versions[i++] = jsonify(version.second);
        }
        return std::move(node);
    }

    Json::Value jsonify(source_root_info &info) {
        Json::Value node;
        node[KEY_BASE_URL] = info._base_url;

        auto &platforms = node[KEY_PLATFORM];

        int i = 0;
        for (auto &platform : info._platforms) {
            platforms[i++] = jsonify(platform.second);
        }
        return std::move(node);
    }
}

/*
 * hiding all details for PODs.
 */

namespace source_dir_impl {
    using namespace csman::core;
    using namespace csman::os;
    using mpp::path_separator;

    // {
    //     "source-url": { source-object },
    //     ...
    // }
    constexpr const char *SOURCE_CACHE_FILE = "sources.json";

    void init(source_dir &dir, const std::string &root_dir) {
        dir._path = root_dir + path_separator + "sources";
        OS::current()->mkdir(dir._path);
    }

    void load(source_dir &dir) {
        std::string source_cache_file = dir._path + path_separator + SOURCE_CACHE_FILE;
        std::fstream stream(source_cache_file, std::ios::in);

        if (!stream.good()) {
            // maybe first run, just return
            return;
        }

        sp<Json::Value> root = load_json_stream(stream);
        auto &sources = *root;

        if (!sources.isObject()) {
            throw_ex("syntax error(source list): source entry should be an object");
        }

        for (auto &source_url : sources.getMemberNames()) {
            auto &source_object = sources[source_url];
            source_root_info info;
            parse_root(info, source_object);
            dir._sources.push_back(info);
        }
    }

    void store(source_dir &dir) {
        Json::Value root;
        for (auto &info : dir._sources) {
            root[info._base_url] = jsonify(info);
        }

        std::string source_cache_file = dir._path + path_separator + SOURCE_CACHE_FILE;
        std::fstream stream(source_cache_file, std::ios::out);

        if (!stream.good()) {
            throw_ex("Failed to open source cache file: %s\n" + source_cache_file);
        }

        save_json_stream(stream, root);
    }

    bool contains(source_dir &dir, const std::string &url) {
        for (auto &source : dir._sources) {
            if (source._base_url == url) {
                return true;
            }
        }
        return false;
    }

    void add_source_info(source_dir &dir, const source_root_info &info) {
        dir._sources.push_back(info);
    }
}

namespace local_package_impl {
    using namespace csman::core;
    using namespace csman::os;
    using mpp::path_separator;

    // <root-dir>/versions/<version-name>/packages/<package-name>/info.json
    // {
    //     "info": { source_package_info },
    //     "files": [ "bin/cs", "bin/cs_dbg", ... ]
    // }
    constexpr const char *PKG_FILE = "info.json";

    constexpr const char *KEY_INFO = "info";
    constexpr const char *KEY_FILES = "files";

    void init(local_package &lp, const std::string &path) {
        lp._path = path;
    }

    void load(local_package &lp) {
        std::string info_file = lp._path + path_separator + PKG_FILE;
        std::fstream stream(info_file, std::ios::in);

        if (!stream.good()) {
            // invalid package, just return
            return;
        }

        sp<Json::Value> root = load_json_stream(stream);
        auto &value = *root;
        auto &info = value[KEY_INFO];
        auto &files = value[KEY_FILES];

        if (!info.isObject() || !files.isArray()) {
            // invalid package, just return
            return;
        }

        // parse package info from source
        parse_package(lp._info, info);

        // parse local file list
        for (auto &f : files) {
            lp._files.push_back(f.asString());
        }
    }

    void store(local_package &lp) {
        Json::Value root;
        root[KEY_INFO] = jsonify(lp._info);

        auto &files = root[KEY_FILES];
        int i = 0;
        for (auto &f : lp._files) {
            files[i++] = f;
        }

        std::string info_file = lp._path + path_separator + PKG_FILE;
        std::fstream stream(info_file, std::ios::out);

        if (!stream.good()) {
            throw_ex("Failed to open package info file: " + info_file);
        }

        save_json_stream(stream, root);
    }
}

namespace local_version_impl {
    using namespace csman::core;
    using namespace csman::os;
    using mpp::path_separator;

    // <root-dir>/versions/<version-name>/bin
    constexpr const char *BIN_DIR = "bin";

    // <root-dir>/versions/<version-name>/lib
    constexpr const char *LIB_DIR = "lib";

    // <root-dir>/versions/<version-name>/include
    constexpr const char *INCLUDE_DIR = "include";

    // <root-dir>/versions/<version-name>/imports
    constexpr const char *IMPORTS_DIR = "imports";

    // <root-dir>/versions/<version-name>/packages
    constexpr const char *PKG_DIR = "packages";

    void init(local_version &lv, const std::string &name,
              const std::string &path) {
        lv._name = name;
        lv._path = path;

        std::string packages_dir = lv._path + path_separator + PKG_DIR;

        for (auto &pkg : OS::current()->ls(packages_dir)) {
            if (pkg._type != file_type::DIR) {
                // skip non-directories
                continue;
            }
            std::string package_dir = packages_dir + path_separator + pkg._name;
            local_package lp;
            local_package_impl::init(lp, package_dir);
            lv._packages.emplace(pkg._name, lp);
        }
    }

    void load(local_version &lv) {
        std::unordered_map<std::string, local_package> checked;
        for (auto &lp : lv._packages) {
            local_package_impl::load(lp.second);
            if (lp.second._info._name.empty()
                || lp.second._info._full_name.empty()
                || lp.first != lp.second._info._name) {
                // skip invalid local package
                continue;
            }

            checked.emplace(lp.first, lp.second);
        }

        lv._packages = std::move(checked);
    }

    void store(local_version &lv) {
        for (auto &lp : lv._packages) {
            local_package_impl::store(lp.second);
        }
    }
}

namespace version_dir_impl {
    using namespace csman::core;
    using namespace csman::os;
    using mpp::path_separator;

    // <root-dir>/versions/current
    // always symbolically linked to the version in use
    constexpr const char *CURRENT_DIR = "current";

    void init(version_dir &dir, const std::string &root_dir) {
        dir._path = root_dir + path_separator + "versions";
        OS::current()->mkdir(dir._path);

        // scan versions directory
        for (auto &version : OS::current()->ls(dir._path)) {
            if (version._type != file_type::DIR) {
                // skip non-directories
                continue;
            }

            if (version._name == CURRENT_DIR) {
                // skip "versions/current"
                continue;
            }

            local_version lv;
            local_version_impl::init(lv, version._name,
                dir._path + path_separator + version._name);
            dir._versions.push_back(lv);
        }
    }

    void load(version_dir &dir) {
        for (auto &lv : dir._versions) {
            local_version_impl::load(lv);
        }
    }

    void store(version_dir &dir) {
        for (auto &lv : dir._versions) {
            local_version_impl::store(lv);
        }
    }
}

namespace user_config_impl {
    using namespace csman::core;
    using namespace csman::os;
    using mpp::path_separator;

    // <root-dir>/config.json
    // {
    //    "key": value,
    //    ...
    // }
    constexpr const char * CONFIG_FILE = "config.json";

    void init(user_config &uc, const std::string &path) {
        uc._path = path + path_separator + CONFIG_FILE;
    }

    void load(user_config &uc) {
        std::fstream stream(uc._path, std::ios::in);
        if (!stream.good()) {
            // no user config
            return;
        }

        sp<Json::Value> root = load_json_stream(stream);
        for (auto &name : root->getMemberNames()) {
            uc._config.emplace(name, (*root)[name].asString());
        }
    }

    void store(user_config &uc) {
        Json::Value root;
        for (auto &c : uc._config) {
            root[c.first] = c.second;
        }

        std::fstream stream(uc._path, std::ios::out);

        if (!stream.good()) {
            throw_ex("Failed to open user config file: " + uc._path);
        }

        save_json_stream(stream, root);
    }

    void set(user_config &uc, const std::string &key, const std::string &value) {
        uc._config[key] = value;
    }

    void unset(user_config &uc, const std::string &key) {
        uc._config.erase(key);
    }

    std::string get(user_config &uc, const std::string &key) {
        auto it = uc._config.find(key);
        if (it == uc._config.end()) {
            return "";
        }
        return it->second;
    }
}

namespace csman {
    namespace core {
        using namespace csman::os;
        using mpp::path_separator;

        /* csman_core */

        void csman_core::load() {
            source_dir_impl::load(_source_dir);
            version_dir_impl::load(_version_dir);
            user_config_impl::load(_user_config);
        }

        void csman_core::store() {
            source_dir_impl::store(_source_dir);
            version_dir_impl::store(_version_dir);
            user_config_impl::store(_user_config);
        }

        void csman_core::init() {
            OS::current()->mkdir(_root_dir);
            source_dir_impl::init(_source_dir, _root_dir);
            version_dir_impl::init(_version_dir, _root_dir);
            user_config_impl::init(_user_config, _root_dir);
        }

        void csman_core::add_source(const std::string &url) {
            if (source_dir_impl::contains(_source_dir, url)) {
                // do not add duplicated source
                return;
            }

            source_updater updater(url);
            updater.update();

            source_dir_impl::add_source_info(_source_dir,
                updater.get_source_info());
        }

        void csman_core::perform(operation &op, bool wait_if_running) {
            if (op._core != nullptr && !wait_if_running) {
                throw_ex("Operation has already got an owner");
            }

            std::lock_guard<std::mutex> guard(op._lock);
            op._core = this;
            op.perform();
            op._core = nullptr;
        }

        void csman_core::set_config(const std::string &key, const std::string &value) {
            user_config_impl::set(_user_config, key, value);
        }

        void csman_core::unset_config(const std::string &key) {
            user_config_impl::unset(_user_config, key);
        }

        std::string csman_core::get_config(const std::string &key) {
            return user_config_impl::get(_user_config, key);
        }

        std::string csman_core::get_platform() {
            return get_config("platform");
        }
    }
}
