//
// Created by kiva on 2020/2/5.
//
#pragma once

#include <string>

namespace csman {
    namespace os {
        class OS {
        public:
            static OS *current();

        public:
            virtual bool mkdir(const std::string &path) = 0;
        };
    }
}
