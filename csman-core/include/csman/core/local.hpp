//
// Created by kiva on 2020/2/5.
//
#pragma once

#include <csman/core/fwd.hpp>
#include <csman/core/source.hpp>
#include <vector>
#include <unordered_map>
#include <utility>

namespace csman {
    namespace core {
        struct source_dir {
            std::string _path;
            std::vector<source_root_info> _sources;
        };

        struct local_package {
            std::string _path;
            source_package_info _info;
            std::vector<std::string> _files;
        };

        struct local_version {
            std::string _path;
            std::string _name;
            std::unordered_map<std::string, local_package> _packages;
        };

        struct version_dir {
            std::string _path;
            std::vector<local_version> _versions;
        };

        class csman_core {
        private:
            /**
             * root directory
             */
            std::string _root_dir;

            /**
             * dir containing source lists.
             */
            source_dir _source_dir;

            /**
             * dir containing installed versions.
             */
            version_dir _version_dir;

        private:
            void init_dir();

        public:
            explicit csman_core(std::string local_dir)
                : _root_dir(std::move(local_dir)) {
                init_dir();
            }

            ~csman_core() = default;

            csman_core(csman_core &&) = default;

            csman_core(const csman_core &) = default;

            csman_core &operator=(csman_core &&) = default;

            csman_core &operator=(const csman_core &) = default;

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
