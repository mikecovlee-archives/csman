//
// Created by kiva on 2020/2/5.
//
#pragma once

#include <csman/core/fwd.hpp>
#include <csman/core/source.hpp>
#include <json/reader.h>

namespace csman {
    namespace core {
        void parse_content(source_content_info &info, Json::Value &value);

        void parse_package(source_package_info &info, Json::Value &value);

        void parse_version(source_version_info &info, Json::Value &value);

        void parse_platform(source_platform_info &info, Json::Value &value);

        void parse_root(source_root_info &info, Json::Value &value);

        void parse_package(source_package_info &info, const std::string &json);

        void parse_version(source_version_info &info, const std::string &json) ;

        void parse_platform(source_platform_info &info, const std::string &json);

        void parse_root(source_root_info &info, const std::string &json);

        void parse_root_url(source_root_info &info, const std::string &url);
    }
}
