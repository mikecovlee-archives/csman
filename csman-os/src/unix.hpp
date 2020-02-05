//
// Created by kiva on 2020/2/5.
//
#pragma once

#ifndef _WIN32

#include <csman/os/os.hpp>

namespace csman {
    namespace os {
        class os_impl_unix : public OS {
        public:
            bool mkdir(const std::string &path) override;

            std::vector<file> ls(const std::string &path) override;
        };
    }
}

#endif
