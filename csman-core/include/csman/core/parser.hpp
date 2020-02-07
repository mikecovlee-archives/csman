//
// Created by kiva on 2020/2/5.
//
#pragma once

#include <csman/core/fwd.hpp>
#include <csman/core/source.hpp>
#include <json/reader.h>

namespace csman {
    namespace core {
        constexpr const char *EXTENSION_PKG = ".cspkg";
        constexpr const char *EXTENSION_DEV = ".csdev";
        constexpr const char *EXTENSION_RTM = ".csrtm";
        constexpr const char *KEY_BASE_URL = "BaseUrl";
        constexpr const char *KEY_PLATFORM = "Platform";
        constexpr const char *KEY_VERSION = "Version";
        constexpr const char *KEY_VERSIONS = "Versions";
        constexpr const char *KEY_LATEST = "Latest";
        constexpr const char *KEY_NIGHTLY = "Nightly";
        constexpr const char *KEY_RTM = "RTM";
        constexpr const char *KEY_DEV = "DEV";
        constexpr const char *KEY_PKG = "PKG";
        constexpr const char *KEY_NAME = "Name";
        constexpr const char *KEY_PNAME = "PackageName";
        constexpr const char *KEY_PFNAME = "PackageFull";
        constexpr const char *KEY_DEPS = "Dependencies";
        constexpr const char *KEY_CONTENTS = "Contents";

        sp<Json::Value> load_json_file(const std::string &path);

        sp<Json::Value> load_json_stream(std::istream &stream);

        sp<Json::Value> load_json(const std::string &json);

        void save_json_stream(std::ostream &stream, Json::Value &value);

        void parse_content(source_content_info &info, Json::Value &value);

        void parse_package(source_package_info &info, Json::Value &value);

        void parse_version(source_version_info &info, Json::Value &value);

        void parse_platform(source_platform_info &info, Json::Value &value);

        void parse_root(source_root_info &info, Json::Value &value);

        void parse_package(source_package_info &info, const std::string &json);

        void parse_version(source_version_info &info, const std::string &json) ;

        void parse_platform(source_platform_info &info, const std::string &json);

        void parse_root(source_root_info &info, const std::string &json);

        std::string jsonify(source_content_type type);

        Json::Value jsonify(const source_content_info &info);

        Json::Value jsonify(const source_package_info &info);

        Json::Value jsonify(const source_version_info &info);

        Json::Value jsonify(const source_platform_info &info);

        Json::Value jsonify(const source_root_info &info);
    }
}
