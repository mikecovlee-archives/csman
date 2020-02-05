//
// Created by kiva on 2020/2/5.
//
#pragma once

#include <memory>
#include <string>
#include <mozart++/string/string.hpp>

namespace csman {
    namespace core {
        constexpr const char *INFO_FILE = "csman.json";

        using mpp::string_ref;

        template <typename T>
        using sp = std::shared_ptr<T>;

        using std::make_shared;

        class source_error : public mpp::runtime_error {
        public:
            source_error() = default;

            explicit source_error(const std::string &str) noexcept
                : mpp::runtime_error(str) {
            }
        };

        void throw_ex(const std::string &what);
    }
}
