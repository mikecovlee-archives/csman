//
// Created by kiva on 2020/2/5.
//
#pragma once

#include <mozart++/optional>
#include <csman/core/core.hpp>
#include <mutex>

namespace csman {
    namespace core {
        // core.hpp
        struct local_package;
        struct local_version;

        struct source_package {
            int _match_rate;
            std::string _owner_version;
            source_package_info _package;
        };

        struct source_version {
            int _match_rate;
            source_version_info _version;
        };

        class operation {
            friend class csman_core;

        private:
            std::mutex _lock;
            csman_core *_core = nullptr;

        private:
            void check_valid_operation();

        protected:
            mpp::optional<std::string> optional_platform();
            std::string requires_platform();

            mpp::optional<std::string> optional_current_version();
            std::string requires_current_version();

            std::vector<source_package> query_package(const std::string &text);
            std::vector<source_version> query_version(const std::string &text);

            std::vector<local_package> query_installed_package(const std::string &text);
            std::vector<local_package> query_installed_package(const std::string &version,
                                                               const std::string &text);
            std::vector<local_version> query_installed_version(const std::string &text);

        public:
            virtual ~operation() = default;
            virtual void perform() = 0;
        };
    }
}

