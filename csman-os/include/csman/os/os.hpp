//
// Created by kiva on 2020/2/5.
//
#pragma once

#include <utility>
#include <vector>
#include <string>

namespace csman {
    namespace os {
        enum class file_type {
            FILE, DIR,
        };

        struct file {
            std::string _name;
            file_type _type;

            file(std::string name, file_type type)
                : _name(std::move(name)), _type(type) {
            }
        };

        class OS {
        public:
            static OS *current();

        public:
            virtual bool mkdir(const std::string &path) = 0;

            virtual std::vector<file> ls(const std::string &path) = 0;

            virtual bool ln(const std::string &target, const std::string &linkpath) = 0;

            virtual bool unlink(const std::string &path) = 0;

            virtual bool file_exists(const std::string &path) = 0;

            virtual bool directory_exists(const std::string &path) = 0;

            virtual bool rm_rf(const std::string &path) = 0;

            virtual std::string error() = 0;

            virtual void rewind_cursor() = 0;

            virtual bool make_executable(const std::string &path);
        };
    }
}
