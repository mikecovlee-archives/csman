//
// Created by kiva on 2020/1/31.
//

#include <set>

#include <fstream>
#include <csman/core/source.hpp>
#include <csman/core/network.hpp>
#include <csman/core/parser.hpp>
#include <json/writer.h>

namespace csman {
    namespace core {
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

        Json::Value jsonify(const source_content_info &info) {
            Json::Value node;
            node[0] = jsonify(info._type);
            node[1] = info._name;
            for (auto &meta : info._meta) {
                node[2][meta.first] = meta.second;
            }
            return std::move(node);
        }

        Json::Value jsonify(const source_package_info &info) {
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

        Json::Value jsonify(const source_version_info &info) {
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

        Json::Value jsonify(const source_platform_info &info) {
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

        Json::Value jsonify(const source_root_info &info) {
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
}

namespace csman {
    namespace core {
        using namespace Json;

        sp<Json::Value> load_json_file(const std::string &path) {
            std::ifstream stream(path, std::ios::in);
            if (!stream.good()) {
                throw_ex("Failed to open file: " + path);
            }

            return load_json_stream(stream);
        }

        sp<Json::Value> load_json_stream(std::istream &stream) {
            sp<Json::Value> value = make_shared<Json::Value>();
            Json::CharReaderBuilder builder;
            std::string errs;
            if (!parseFromStream(builder, stream, value.get(), &errs)) {
                throw_ex("Failed to load json: " + errs);
            }

            return value;
        }

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

        void save_json_stream(std::ostream &stream, Json::Value &value) {
            sp<Json::StreamWriter> writer(Json::StreamWriterBuilder().newStreamWriter());
            writer->write(value, &stream);
        }

        source_content_type parse_type(string_ref type) {
            if (type.equals_ignore_case("ZIP")) {
                return source_content_type::ZIP;
            } else if (type.equals_ignore_case("CSE")) {
                return source_content_type::CSE;
            } else if (type.equals_ignore_case("CSP")) {
                return source_content_type::CSP;
            } else if (type.equals_ignore_case("BIN")) {
                return source_content_type::BIN;
            } else if (type.equals_ignore_case("DLL")) {
                return source_content_type ::DLL;
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
            int meta_index = -1;

            switch (value.size()) {
                case 0: {
                    throw_ex("syntax error(content): content should have "
                             "at least one element(namely the name element)");
                    break;
                }

                case 1: {
                    // we assume that the only provided element is content name,
                    // so we will infer its type according to file extension.
                    info._name = value[0].asString();
                    info._type = source_content_type::UNKNOWN;
                    // no meta absolutely.
                    break;
                }

                case 2: {
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
                    break;
                }

                case 3: {
                    // thanks to god, the source provided all.
                    info._type = parse_type(value[0].asString());
                    info._name = value[1].asString();
                    meta_index = 2;
                    break;
                }

                default: {
                    throw_ex("syntax error(content): content should have "
                             "at most 3 elements(name, type and meta)");
                    break;
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
                throw_ex("syntax error(package: " + info._full_name
                         + "): Contents entry should be an array");
            }

            // rewrite base url if source defined
            if (!value[KEY_BASE_URL].asString().empty()) {
                info._base_url = value[KEY_BASE_URL].asString();
            }

            // rewrite package name if source defined
            // Note: we should use KEY_PNAME because KEY_NAME is display name
            if (!value[KEY_PNAME].asString().empty()) {
                info._name = value[KEY_PNAME].asString();
            }

            // rewrite package full name if source defined
            // Note: we should use KEY_PFNAME because KEY_NAME is display name
            if (!value[KEY_PFNAME].asString().empty()) {
                info._full_name = value[KEY_PFNAME].asString();
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

            // rewrite name if source defined
            if (!value[KEY_NAME].asString().empty()) {
                info._name = value[KEY_NAME].asString();
            }

            info._package_rtm = value[KEY_RTM].asString();
            info._package_dev = value[KEY_DEV].asString();

            // no package to parse, just return
            if (pkgs.empty()) {
                return;
            }

            if (pkgs[0].isObject()) {
                // PKG contain inline package object
                for (auto &pkg : pkgs) {
                    source_package_info package_info{};
                    parse_package(package_info, pkg);
                    if (package_info._name.empty() || package_info._full_name.empty()) {
                        throw_ex("syntax error(package): inline object should contain a package name "
                                 "and a package full name");
                    }

                    // inherit base url
                    if (package_info._base_url.empty()) {
                        // base url should be constructed with package full name
                        package_info._base_url = info._base_url + "/" + package_info._full_name;
                    }

                    // package was indexed by package name (not full name or display name)
                    info._packages.emplace(package_info._name, package_info);
                }

            } else {
                // PKG contain only package name

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
                    // Inherit default values
                    package_info._base_url = package_url;
                    package_info._name = pkg.first;
                    package_info._full_name = pkg.second;
                    parse_package(package_info, package_json);

                    // put parsed version into platform config
                    // DO NOT use full name
                    info._packages.emplace(pkg.first, package_info);
                }
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

            // rewrite name if source defined
            if (!value[KEY_NAME].asString().empty()) {
                info._name = value[KEY_NAME].asString();
            }

            info._version_default = value[KEY_VERSION].asString();
            info._version_latest = value[KEY_LATEST].asString();
            info._version_nightly = value[KEY_NIGHTLY].asString();

            if (value.isMember(KEY_VERSIONS)) {
                // source has inline version list
                auto &versions = value[KEY_VERSIONS];
                for (auto &version : versions) {
                    source_version_info version_info{};
                    parse_version(version_info, version);
                    if (version_info._name.empty()) {
                        throw_ex("syntax error(version): inline object should contain a name");
                    }

                    // inherit base url
                    if (version_info._base_url.empty()) {
                        version_info._base_url = info._base_url + "/" + version_info._name;
                    }

                    // put loaded inline object
                    info._versions.emplace(version_info._name, version_info);
                }

            } else {
                // source doesn't have version list

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

            for (auto &platform : platforms) {
                if (platform.isObject()) {
                    // if platform is an inline object,
                    // just parse it!
                    source_platform_info platform_info{};
                    parse_platform(platform_info, platform);
                    if (platform_info._name.empty()) {
                        throw_ex("syntax error(platform): inline object should contain a name");
                    }

                    // inherit base url
                    if (platform_info._base_url.empty()) {
                        platform_info._base_url = info._base_url + "/" + platform_info._name;
                    }

                    // put loaded inline object
                    info._platforms.emplace(platform_info._name, platform_info);
                    continue;
                }

                // otherwise, we need to request the distributed description file
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
                // Inherit default values
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
    }
}
