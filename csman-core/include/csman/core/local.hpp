//
// Created by kiva on 2020/2/5.
//
#pragma once

#include <csman/core/fwd.hpp>
#include <csman/core/source.hpp>
#include <vector>
#include <utility>

namespace csman {
    namespace core {
        struct source_dir {
            std::string _path;
            std::vector<source_root_info> _sources;

            void init(const std::string &root_dir);

            void load();

            void store();
        };

        class csman_config {
        private:
            /**
             * root directory
             */
            std::string _root_dir;

            /**
             * dir containing source lists.
             */
            source_dir _source_dir;

        private:
            void init_dir();

        public:
            explicit csman_config(std::string local_dir)
                : _root_dir(std::move(local_dir)) {
                init_dir();
            }

            ~csman_config() = default;

            csman_config(csman_config &&) = default;

            csman_config(const csman_config &) = default;

            csman_config &operator=(csman_config &&) = default;

            csman_config &operator=(const csman_config &) = default;

        public:
            const std::string &get_local_dir() const {
                return _root_dir;
            }

            void add_source(const std::string &url);

            void load();

            void store();
        };
    }
}
