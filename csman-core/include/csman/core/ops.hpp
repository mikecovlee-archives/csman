//
// Created by kiva on 2020/2/5.
//
#pragma once

#include <mozart++/optional>
#include <csman/core/core.hpp>
#include <mutex>

namespace csman {
    namespace core {
        struct query_result {
            int match_rate;
            source_package_info _package;
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

            std::vector<query_result> query_package(const std::string &text);

        public:
            virtual void perform() = 0;
        };
    }
}

