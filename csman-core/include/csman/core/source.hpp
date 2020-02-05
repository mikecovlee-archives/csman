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
            UNKNOWN, BIN, DLL, CSE, CSP, ZIP,
        };

        struct source_content_info final {
            source_content_type _type = source_content_type::UNKNOWN;
            std::string _name;
            std::unordered_map<std::string, std::string> _meta;
        };

        struct source_package_info final {
            std::string _name;
            std::string _base_url;
            std::string _full_name;
            std::string _display_name;
            std::string _version;
            std::unordered_map<std::string, std::string> _deps;
            std::vector<source_content_info> _contents;
        };

        struct source_version_info final {
            std::string _name;
            std::string _base_url;
            std::string _package_rtm;
            std::string _package_dev;
            std::unordered_map<std::string, source_package_info> _packages;
        };

        struct source_platform_info final {
            std::string _name;
            std::string _base_url;
            std::string _version_default;
            std::string _version_latest;
            std::string _version_nightly;
            std::unordered_map<std::string, source_version_info> _versions;
        };

        struct source_root_info final {
            std::string _base_url;
            std::unordered_map<std::string, source_platform_info> _platforms;
        };

        class source_updater final {
        private:
            std::string _source_url;
            source_root_info _info;

        public:
            explicit source_updater(std::string url)
                : _source_url(std::move(url)) {
            }

            ~source_updater() = default;

            source_updater(const source_updater &) = default;

            source_updater(source_updater &&) = default;

            source_updater &operator=(const source_updater &) = default;

            source_updater &operator=(source_updater &&) = default;

        public:
            void update();

            const source_root_info& get_source_info() const {
                return _info;
            }
        };
    }
}

