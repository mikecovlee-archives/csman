//
// Created by kiva on 2020/1/31.
//

#include <csman/os/os.hpp>
#include <csman/core/local.hpp>
#include <csman/core/parser.hpp>
#include <json/writer.h>
#include <fstream>

namespace source_dir_impl {
    using namespace csman::core;
    using mpp::path_separator;

    // {
    //     "source-url": { source-object },
    //     ...
    // }
    constexpr const char *SOURCE_CACHE_FILE = "sources.json";

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

        sp<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
        writer->write(root, &stream);
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

namespace csman {
    namespace core {
        using csman::os::OS;
        using mpp::path_separator;

        /* csman_core */

        void csman_core::load() {
            _source_dir.load();
        }

        void csman_core::store() {
            _source_dir.store();
        }

        void csman_core::init_dir() {
            OS::current()->mkdir(_root_dir);
            _source_dir.init(_root_dir);
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

        /* source_dir */

        void source_dir::init(const std::string &root_dir) {
            _path = root_dir + path_separator + "sources";
            OS::current()->mkdir(_path);
        }

        void source_dir::load() {
            source_dir_impl::load(*this);
        }

        void source_dir::store() {
            source_dir_impl::store(*this);
        }
    }
}
