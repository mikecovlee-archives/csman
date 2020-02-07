//
// Created by kiva on 2020/2/5.
//
#ifndef _WIN32

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

#include "unix.hpp"

namespace csman {
    namespace os {
        bool os_impl_unix::mkdir(const std::string &path) {
            return ::mkdir(path.c_str(), 0755) == 0;
        }

        std::vector<file> os_impl_unix::ls(const std::string &path) {
            DIR *dp = nullptr;
            if ((dp = ::opendir(path.c_str())) == nullptr) {
                return std::vector<file>();
            }

            std::vector<file> files;

            dirent *ent = nullptr;
            while ((ent = ::readdir(dp)) != nullptr) {
                files.emplace_back(ent->d_name,
                    ent->d_type == DT_DIR ? file_type::DIR : file_type::FILE);
            }

            ::closedir(dp);
            return std::move(files);
        }

        void os_impl_unix::rewind_cursor() {
            putchar('\r');
        }

        bool os_impl_unix::ln(const std::string &target, const std::string &linkpath) {
            unlink(linkpath);
            return ::symlink(target.c_str(), linkpath.c_str()) == 0;
        }

        bool os_impl_unix::file_exists(const std::string &path) {
            struct stat buf{};
            if (::stat(path.c_str(), &buf) == 0) {
                return S_ISDIR(buf.st_mode);
            }
            return false;
        }

        bool os_impl_unix::directory_exists(const std::string &path) {
            struct stat buf{};
            if (::stat(path.c_str(), &buf) == 0) {
                return !S_ISDIR(buf.st_mode);
            }
            return false;
        }

        std::string os_impl_unix::error() {
            return ::strerror(errno);
        }

        bool os_impl_unix::unlink(const std::string &path) {
            return ::unlink(path.c_str()) == 0;
        }
    }
}

#endif
