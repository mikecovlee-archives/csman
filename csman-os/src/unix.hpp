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

            void rewind_cursor() override;

            std::string error() override;

            bool rm_rf(const std::string &path) override;

            bool unlink(const std::string &path) override;

            bool file_exists(const std::string &path) override;

            bool directory_exists(const std::string &path) override;

            bool make_executable(const std::string &path) override;

            bool ln(const std::string &target, const std::string &linkpath) override;
        };
    }
}

#endif
