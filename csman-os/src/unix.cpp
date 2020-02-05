//
// Created by kiva on 2020/2/5.
//
#ifndef _WIN32

#include <sys/stat.h>
#include <dirent.h>

#include "unix.hpp"

namespace csman {
    namespace os {
        bool os_impl_unix::mkdir(const std::string &path) {
            return ::mkdir(path.c_str(), 0755) == 0;
        }

        std::vector<file> os_impl_unix::ls(const std::string &path) {
            DIR *dp = nullptr;
            if ((dp = opendir(path.c_str())) == nullptr) {
                return std::vector<file>();
            }

            std::vector<file> files;

            dirent *ent = nullptr;
            while ((ent = readdir(dp)) != nullptr) {
                files.emplace_back(ent->d_name,
                    ent->d_type == DT_DIR ? file_type::DIR : file_type::FILE);
            }

            closedir(dp);
            return std::move(files);
        }
    }
}

#endif
