//
// Created by kiva on 2020/2/5.
//

#include <array>
#include <csman/core/ops.hpp>

namespace csman {
    namespace core {
        void operation::check_valid_operation() {
            if (_core == nullptr) {
                throw_ex("Invalid operation: operation was not performed "
                         "by csman_core::perform()");
            }
        }

        mpp::optional<std::string> operation::optional_platform() {
            auto &&platform = _core->get_platform();
            if (platform.empty()) {
                return mpp::optional<std::string>::none();
            }
            return mpp::optional<std::string>::from(platform);
        }

        std::string operation::requires_platform() {
            auto &&platform = optional_platform();
            if (!platform.has_value()) {
                throw_ex("platform not found in config.json, please run "
                         "`csman config set platform <your-platform>` first.");
            }
            return platform.get();
        }

        mpp::optional<std::string> operation::optional_current_version() {
            auto &&version = _core->get_current_version();
            if (version.empty()) {
                return mpp::optional<std::string>::none();
            }
            return mpp::optional<std::string>::from(version);
        }

        std::string operation::requires_current_version() {
            auto &&version = optional_current_version();
            if (!version.has_value()) {
                throw_ex("current version not set, please run "
                         "`csman checkout <version>` first.");
            }
            return version.get();
        }
    }
}
