//
// Created by kiva on 2020/2/5.
//
#ifndef _WIN32

#include <sys/stat.h>

#include "unix.hpp"

namespace csman {
    namespace os {
        bool os_impl_unix::mkdir(const std::string &path) {
            return ::mkdir(path.c_str(), 0755) == 0;
        }
    }
}

#endif
