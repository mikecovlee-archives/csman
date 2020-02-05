//
// Created by kiva on 2020/2/5.
//

#include <array>
#include <vector>
#include <algorithm>
#include <csman/core/ops.hpp>

namespace {
    int edit_distance(const std::string &lhs, const std::string &rhs) {
        if (lhs.empty() || rhs.empty()) {
            return std::max(lhs.length(), lhs.length());
        }

        std::array<std::vector<int>, 2> dp{
            std::vector<int>(rhs.length() + 1),
            std::vector<int>(rhs.length() + 1),
        };

        for (int i = 0; i <= rhs.length(); ++i) {
            dp[0][i] = i;
        }

        int last_use = 0;
        int current = 1;
        for (int i = 1; i <= lhs.length(); ++i) {
            dp[current][0] = i;

            for (int j = 1; j <= rhs.length(); ++j) {
                dp[current][j] = std::min(dp[last_use][j], dp[current][j - 1]) + 1;
                dp[current][j] = std::min(dp[current][j],
                    dp[last_use][j - 1] + (lhs[i - 1] == rhs[j - 1] ? 0 : 1));
            }

            std::swap(last_use, current);
        }

        return dp[last_use][rhs.length()];
    }

    int mix_match_rate(int name, int display_name) {
        static constexpr int name_weight = 1;
        static constexpr int display_name_weight = 2;
        return name * name_weight
               + display_name * display_name_weight;
    }
}

namespace {
    using namespace csman::core;

    constexpr int MAX_RATE = 2048;

    void search_in_version(std::vector<query_result> &result,
                           const std::string &text,
                           const source_version_info &info) {
        for (auto &pkg : info._packages) {
            // contains first
            if (mpp::string_ref(pkg.second._name).contains_ignore_case(text)
                || mpp::string_ref(pkg.second._display_name).contains_ignore_case(text)) {
                // match rate second
                int rate = mix_match_rate(edit_distance(text, pkg.second._name),
                    edit_distance(text, pkg.second._display_name));

                // MAX_RATE is almost impossible
                // but when it happens, the match must be ignored
                if (rate > MAX_RATE) {
                    continue;
                }

                query_result r;
                r._package = pkg.second;
                r.match_rate = rate;
                result.push_back(r);
            }
        }
    }
}

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
            // TODO: obtain current version
            return mpp::optional<std::string>::none();
        }

        std::string operation::requires_current_version() {
            auto &&version = optional_current_version();
            if (!version.has_value()) {
                throw_ex("current version not set, please run "
                         "`csman checkout <version>` first.");
            }
            return version.get();
        }

        std::vector<query_result> operation::query_package(const std::string &text) {
            check_valid_operation();

            auto &&platform = requires_platform();
            auto &&version = optional_current_version();
            std::vector<query_result> result;

            // search each source
            for (auto &source : _core->_source_dir._sources) {
                // only search current platform
                // no why
                auto it = source._platforms.find(platform);
                if (it == source._platforms.end()) {
                    // try next source
                    continue;
                }

                auto &platform_info = it->second;
                if (version.has_value()) {
                    // if current version was set
                    // we only search in current version
                    auto ver = platform_info._versions.find(version.get());
                    if (ver != platform_info._versions.end()) {
                        search_in_version(result, text, ver->second);
                    }
                } else {
                    // otherwise, we search in all versions,
                    // in case that the user wants to know which version
                    // has his/her wanted package.
                    for (auto &e : platform_info._versions) {
                        search_in_version(result, text, e.second);
                    }
                }
            }

            return std::move(result);
        }
    }
}
