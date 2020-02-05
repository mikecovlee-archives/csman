//
// Created by kiva on 2020/2/5.
//
#pragma once

#include <csman/core/fwd.hpp>

#include <string>
#include <vector>
#include <unordered_map>

namespace csman {
    namespace core {
        enum class source_content_type {
            UNKNOWN, BIN, CSE, ZIP,
        };

        //    "Contents": [
        //        [
        //            "ZIP",
        //            "contents.zip",
        //            {
        //                "HEADER": "include",
        //                "LIB": "lib"
        //            }
        //        ]
        //    ]
        //
        struct source_content_info {
            source_content_type _type;
            std::string _name;
            std::unordered_map<std::string, std::string> _meta;
        };

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
        struct source_package_info {
            std::string _package_name;
            std::string _name;
            std::string _version;
            std::unordered_map<std::string, std::string> _deps;
            std::vector<source_content_info> _contents;
        };

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
        struct source_version_info {
            std::string _name;
            std::string _base_url;
            std::string _package_runtime;
            std::string _package_dev;
            std::vector<source_package_info> _packages;
        };

        // example csman.json:
        // {
        //    "Version": "3.3.3.7",
        //    "Latest": "3.3.3.7",
        //    "Nightly": "3.3.3.7"
        // }
        struct source_platform_info {
            std::string _name;
            std::string _base_url;
            std::string _version_default;
            std::string _version_latest;
            std::string _version_nightly;
            std::vector<source_version_info> _versions;
        };

        // example csinfo.json:
        // {
        //    "BaseUrl": "http://mirrors.covariant.cn/csman/",
        //    "Platform": [
        //        "Linux_GCC_AMD64",
        //        "Win32_MinGW-w64_i386",
        //        "Win32_MinGW-w64_AMD64"
        //    ]
        // }
        struct source_root_info {
            std::string _base_url;
            std::vector<source_platform_info> _platforms;
        };
    }
}

