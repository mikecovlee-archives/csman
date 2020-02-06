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
            std::string _current;
            std::vector<local_version> _versions;
        };

        struct user_config {
            std::string _path;
            std::unordered_map<std::string, std::string> _config;
        };

        class operation;

        class csman_core {
            friend class operation;

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

            /**
             * user config file
             */
            user_config _user_config;

        private:
            void init();

        public:
            explicit csman_core(std::string local_dir)
                : _root_dir(std::move(local_dir)) {
                init();
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

            void set_config(const std::string &key, const std::string &value);

            std::string get_config(const std::string &key);

            void unset_config(const std::string &key);

            std::string get_platform();

            std::string get_current_version();

            void set_current_version(const std::string &version);

            void load();

            void store();

            void perform(operation &op, bool wait_if_running = false);
        };
    }
}
