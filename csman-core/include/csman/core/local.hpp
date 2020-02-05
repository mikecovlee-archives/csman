//
// Created by kiva on 2020/2/5.
//
#pragma once

#include <csman/core/fwd.hpp>
#include <utility>

namespace csman {
    namespace core {
        class csman_config {
        private:
            std::string _local_dir;

        public:
            explicit csman_config(std::string local_dir)
                : _local_dir(std::move(local_dir)) {
            }

            ~csman_config() = default;

            csman_config(csman_config &&) = default;

            csman_config(const csman_config &) = default;

            csman_config &operator=(csman_config &&) = default;

            csman_config &operator=(const csman_config &) = default;

        public:
            const std::string &get_local_dir() const {
                return _local_dir;
            }

            void load();
        };
    }
}
