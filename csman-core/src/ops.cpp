//
// Created by kiva on 2020/2/5.
//

#include <array>
#include <csman/core/ops.hpp>

namespace csman {
    namespace core {
        void operation::check_valid_operation() {
            static std::string ERROR =
                "Internal error: operations should be performed through csman_core::perform()";
            if (_core == nullptr) {
                throw_ex(ERROR);
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
            static std::string ERROR =
                "platform not set, you can run the following command\n"
                "\n"
                "    csman config set platform <your-platform>\n"
                "\n"
                "to tell me which platform you are using.\n";

            auto &&platform = optional_platform();
            if (!platform.has_value()) {
                throw_ex(ERROR);
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
            static std::string ERROR =
                "current version not set, you can run the following command\n"
                "\n"
                "    csman checkout <version>\n"
                "\n"
                "to set current version to <version>.\n";

            auto &&version = optional_current_version();
            if (!version.has_value()) {
                throw_ex(ERROR);
            }
            return version.get();
        }
    }
}
