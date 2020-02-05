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
        };
    }
}
