//
// Created by kiva on 2020/1/31.
//

#include <set>

#include <csman/core/source.hpp>
#include <csman/core/network.hpp>
#include <csman/core/parser.hpp>

namespace csman {
    namespace core {
        using namespace Json;

        constexpr const char *INFO_FILE = "csman.json";
        constexpr const char *EXTENSION_PKG = ".cspkg";
        constexpr const char *EXTENSION_DEV = ".csdev";
        constexpr const char *EXTENSION_RTM = ".csrtm";
        constexpr const char *KEY_BASE_URL = "BaseUrl";
        constexpr const char *KEY_PLATFORM = "Platform";
        constexpr const char *KEY_VERSION = "Version";
        constexpr const char *KEY_LATEST = "Latest";
        constexpr const char *KEY_NIGHTLY = "Nightly";
        constexpr const char *KEY_RTM = "RTM";
        constexpr const char *KEY_DEV = "DEV";
        constexpr const char *KEY_PKG = "PKG";
        constexpr const char *KEY_NAME = "Name";
        constexpr const char *KEY_DEPS = "Dependencies";
        constexpr const char *KEY_CONTENTS = "Contents";

        sp<Json::Value> load_json(const std::string &json) {
            const char *text = json.c_str();

            sp<Json::Value> value = make_shared<Json::Value>();
            sp<CharReader> reader(CharReaderBuilder().newCharReader());

            std::string err;
            if (!reader->parse(text, text + json.length(), value.get(), &err)) {
                throw_ex("Failed to load json: " + err);
            }

            return value;
        }

        source_content_type parse_type(string_ref type) {
            if (type.equals("ZIP")) {
                return source_content_type::ZIP;
            } else if (type.equals("CSE")) {
                return source_content_type::CSE;
            } else if (type.equals_ignore_case("BIN")) {
                return source_content_type::BIN;
            }

            return source_content_type::UNKNOWN;
        }

        source_content_type try_infer_type(string_ref name) {
            auto index = name.rfind('.');
            if (index != string_ref::npos) {
                return parse_type(name.substr(index + 1));
            }
            return source_content_type::UNKNOWN;
        }

        // [
        //     "ZIP",
        //     "contents.zip",
        //     {
        //         "HEADER": "include",
        //         "LIB": "lib"
        //     }
        // ]
        void parse_content(source_content_info &info, Json::Value &value) {
            if (value.empty()) {
                throw_ex("syntax error(content): content should have "
                         "at least one element(namely the name element)");
            }

            int meta_index = -1;

            if (value.size() == 1) {
                // we assume that the only provided element is content name,
                // so we will infer its type according to file extension.
                info._name = value[0].asString();
                info._type = source_content_type::UNKNOWN;
                // no meta absolutely.
            } else {
                // here, the source provided at least name,
                // and optionally type or meta

                if (value[1].isObject()) {
                    // now, the source provided name and meta
                    info._name = value[0].asString();
                    info._type = source_content_type::UNKNOWN;
                    meta_index = 1;
                } else {
                    // now the source provided name and type
                    info._type = parse_type(value[0].asString());
                    info._name = value[1].asString();
                }
            }

            // try to infer type form file extension
            if (info._type == source_content_type::UNKNOWN) {
                info._type = try_infer_type(info._name);
                if (info._type == source_content_type::UNKNOWN) {
                    throw_ex("syntax error(content: " + info._name
                             + "): cannot infer content type from name, "
                               "please point it out explicitly");
                }
            }

            // parse meta if defined
            if (meta_index > 0 && meta_index < value.size()) {
                auto &metas = value[meta_index];

                for (auto &name : metas.getMemberNames()) {
                    info._meta.emplace(name, metas[name].asString());
                }
            }
        }

        // example csman.json
        // {
        //    "Name": "Official Build System for CovScript",
        //    "Version": "3.3.3.7",
        //    "Dependencies": {
        //        "cs.runtime": "3.3.3.7",
        //        "cs.develop": "3.3.3.7"
        //    },
        //    "Contents": [
        //        [
        //            "BIN",
        //            "csbuild"
        //        ]
        //    ]
        // }
        void parse_package(source_package_info &info, Json::Value &value) {
            auto &deps = value[KEY_DEPS];
            auto &contents = value[KEY_CONTENTS];

            if (!contents.isArray()) {
                throw_ex("syntax error(package: " + info._name
                         + "): Contents entry should be an array");
            }

            // rewrite base url if source defined
            if (!value[KEY_BASE_URL].asString().empty()) {
                info._base_url = value[KEY_BASE_URL].asString();
            }

            info._display_name = value[KEY_NAME].asString();
            info._version = value[KEY_VERSION].asString();

            // parse contents
            for (auto &content : contents) {
                if (!content.isArray()) {
                    throw_ex("syntax error(package: " + info._name
                             + "): Contents elements should be arrays");
                }

                source_content_info content_info;
                parse_content(content_info, content);
                info._contents.push_back(content_info);
            }

            // only load dependency names and versions,
            // resolve relationship between them is not parser's case.
            for (auto &name : deps.getMemberNames()) {
                info._deps.emplace(name, deps[name].asString());
            }
        }

        void parse_package(source_package_info &info, const std::string &json) {
            sp<Json::Value> root = load_json(json);
            auto &value = *root.get();
            parse_package(info, value);
        }

        // example csman.json:
        // {
        //    "RTM": "cs.runtime",
        //    "DEV": "cs.develop",
        //    "PKG": [
        //        "cics.codec",
        //        "cics.csbuild",
        //        "cics.darwin",
        //        "cics.imgui",
        //        "cics.network",
        //        "cics.regex",
        //        "cics.sqlite",
        //        "cics.streams"
        //    ]
        // }
        void parse_version(source_version_info &info, Json::Value &value) {
            auto &pkgs = value[KEY_PKG];

            if (!pkgs.isArray()) {
                throw_ex("syntax error(version: " + info._name
                         + "): PKG entry should be an array");
            }

            // rewrite base url if source defined
            if (!value[KEY_BASE_URL].asString().empty()) {
                info._base_url = value[KEY_BASE_URL].asString();
            }

            info._package_rtm = value[KEY_RTM].asString();
            info._package_dev = value[KEY_DEV].asString();

            // remove duplicated packages,
            // so the same package will be parsed only once.
            std::unordered_map<std::string, std::string> names;
            names.emplace(info._package_rtm, info._package_rtm + EXTENSION_RTM);
            names.emplace(info._package_dev, info._package_dev + EXTENSION_DEV);
            for (auto &pkg : pkgs) {
                auto &&name = pkg.asString();
                names.emplace(name, name + EXTENSION_PKG);
            }

            // parse it!
            for (auto &pkg : names) {
                // pkg.first : package name
                // pkg.second: package full name (with extension)

                std::string package_url = info._base_url + "/" + pkg.second;
                std::string package_info_url = package_url + "/" + INFO_FILE;

                // request csman.json for every version
                std::string package_json;
                if (!network::get_url_text(package_info_url, package_json)) {
                    throw_ex("Failed to get file: " + package_info_url);
                }

                // parse it!
                source_package_info package_info;
                // Inherit default base url
                package_info._base_url = package_url;
                package_info._name = pkg.first;
                package_info._full_name = pkg.second;
                parse_package(package_info, package_json);

                // put parsed version into platform config
                // DO NOT use full name
                info._packages.emplace(pkg.first, package_info);
            }
        }

        void parse_version(source_version_info &info, const std::string &json) {
            sp<Json::Value> root = load_json(json);
            auto &value = *root.get();
            parse_version(info, value);
        }

        // example csman.json:
        // {
        //    "Version": "3.3.3.7",
        //    "Latest": "3.3.3.7",
        //    "Nightly": "3.3.3.7"
        // }
        void parse_platform(source_platform_info &info, Json::Value &value) {
            // rewrite base url if source defined
            if (!value[KEY_BASE_URL].asString().empty()) {
                info._base_url = value[KEY_BASE_URL].asString();
            }

            info._version_default = value[KEY_VERSION].asString();
            info._version_latest = value[KEY_LATEST].asString();
            info._version_nightly = value[KEY_NIGHTLY].asString();

            // remove duplicated version names,
            // so the same version will be parsed only once.
            std::set<std::string> names{
                info._version_default,
                info._version_latest,
                info._version_nightly
            };

            for (auto &name : names) {
                std::string version_url = info._base_url + "/" + name;
                std::string version_info_url = version_url + "/" + INFO_FILE;

                // request csman.json for every version
                std::string version_json;
                if (!network::get_url_text(version_info_url, version_json)) {
                    throw_ex("Failed to get file: " + version_info_url);
                }

                // parse it!
                source_version_info version_info;
                // Inherit default base url
                version_info._base_url = version_url;
                version_info._name = name;
                parse_version(version_info, version_json);

                // put parsed version into platform config
                info._versions.emplace(name, version_info);
            }
        }

        void parse_platform(source_platform_info &info, const std::string &json) {
            sp<Json::Value> root = load_json(json);
            auto &value = *root.get();
            parse_platform(info, value);
        }

        // example csinfo.json:
        // {
        //    "BaseUrl": "http://mirrors.covariant.cn/csman/",
        //    "Platform": [
        //        "Linux_GCC_AMD64",
        //        "Win32_MinGW-w64_i386",
        //        "Win32_MinGW-w64_AMD64"
        //    ]
        // }
        void parse_root(source_root_info &info, Json::Value &value) {
            auto &platforms = value[KEY_PLATFORM];

            if (!platforms.isArray()) {
                throw_ex("syntax error(root): platform entry should be an array");
            }

            // rewrite base url if source defined
            if (!value[KEY_BASE_URL].asString().empty()) {
                info._base_url = value[KEY_BASE_URL].asString();
            }

            for (const auto &platform : platforms) {
                auto &&platform_name = platform.asString();

                std::string platform_url = info._base_url + "/" + platform_name;
                std::string platform_info_url = platform_url + "/" + INFO_FILE;

                // request csman.json for every platform
                std::string platform_json;
                if (!network::get_url_text(platform_info_url, platform_json)) {
                    throw_ex("Failed to get file: " + platform_info_url);
                }

                // parse it!
                source_platform_info platform_info;
                // Inherit default base url
                platform_info._base_url = platform_url;
                platform_info._name = platform_name;
                parse_platform(platform_info, platform_json);

                // put parsed platform into root config
                info._platforms.emplace(platform_name, platform_info);
            }
        }

        void parse_root(source_root_info &info, const std::string &json) {
            sp<Json::Value> root = load_json(json);
            auto &value = *root.get();
            parse_root(info, value);
        }

        void parse_root_url(source_root_info &info, const std::string &url) {
            std::string json;
            std::string root_info_url = url + "/" + INFO_FILE;

            if (!network::get_url_text(root_info_url, json)) {
                throw_ex("Failed to get file: " + root_info_url
                         + ": " + json);
            }

            // Inherit base url from parent.
            // For root config, the parent is user.
            info._base_url = url;
            parse_root(info, json);
        }
    }
}
