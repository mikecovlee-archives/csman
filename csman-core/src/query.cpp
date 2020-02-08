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
    constexpr int MAX_VERSION_RATE = 6;

    int package_match_rate(const std::string &text,
                           const std::string &name,
                           const std::string &display_name) {
        return mix_match_rate(edit_distance(text, name),
            edit_distance(text, display_name));
    }

    bool package_matches(const std::string &text,
                         const std::string &name,
                         const std::string &display_name,
                         int *rate = nullptr) {
        if (mpp::string_ref(name).contains_ignore_case(text)
            || mpp::string_ref(display_name).contains_ignore_case(text)) {
            // match rate second
            int rate_ = package_match_rate(text, name, display_name);
            if (rate != nullptr) {
                *rate = rate_;
            }

            // MAX_RATE is almost impossible
            // but when it happens, the match must be ignored
            if (rate_ <= MAX_RATE) {
                return true;
            }
        }
        return false;
    }

    bool version_matches(const std::string &text,
                         const std::string &version,
                         int *rate = nullptr) {
        int rate_ = edit_distance(text, version);
        if (rate != nullptr) {
            *rate = rate_;
        }
        return rate_ <= MAX_VERSION_RATE;
    }
}

namespace csman {
    namespace core {
        std::vector<source_package> operation::query_package(const std::string &text) {
            check_valid_operation();

            auto &&platform = requires_platform();
            auto &&version = optional_current_version();
            std::vector<source_package> result;

            // if current version was set
            // we only search in current version.
            // otherwise, we search in all versions,
            // in case that the user wants to know which version
            // has his/her wanted package.
            bool all_version = !version.has_value();

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
                for (auto &ver : platform_info._versions) {
                    if (!all_version && ver.first != version.get()) {
                        // if the user doesn't want query in all versions,
                        // and we are not in the current version,
                        // just skip and try next.
                        continue;
                    }

                    // enumerate each package
                    for (auto &pkg : ver.second._packages) {
                        int rate = 0;
                        auto &info = pkg.second;
                        if (package_matches(text, info._name, info._display_name, &rate)) {
                            source_package r;
                            r._owner_version = ver.first;
                            r._package = pkg.second;
                            r._match_rate = rate;
                            result.push_back(r);
                        }
                    }
                }
            }

            return std::move(result);
        }

        std::vector<source_version> operation::query_version(const std::string &text) {
            check_valid_operation();

            auto &&platform = requires_platform();
            std::vector<source_version> result;

            bool all_version = string_ref("all").equals_ignore_case(text);

            for (auto &source : _core->_source_dir._sources) {
                auto it = source._platforms.find(platform);
                if (it == source._platforms.end()) {
                    // try next source
                    continue;
                }

                auto &platform_info = it->second;
                for (auto &ver : platform_info._versions) {
                    int rate = 0;
                    if (all_version || version_matches(text, ver.second._name, &rate)) {
                        source_version sv;
                        sv._match_rate = rate;
                        sv._version = ver.second;
                        result.push_back(sv);
                    }
                }
            }

            return std::move(result);
        }

        std::vector<local_package> operation::query_installed_package(const std::string &text) {
            check_valid_operation();
            return query_installed_package(requires_current_version(), text);
        }

        std::vector<local_package> operation::query_installed_package(const std::string &version,
                                                                      const std::string &text) {
            check_valid_operation();
            auto &&versions = query_installed_version(version);
            std::vector<local_package> result;

            bool all_package = string_ref("all").equals_ignore_case(text);

            for (auto &ver : versions) {
                if (ver._name != version) {
                    // there might be similar versions installed,
                    // but we only want the current one.
                    continue;
                }

                // enumerate each package
                for (auto &pkg : ver._packages) {
                    auto &info = pkg.second._info;
                    if (all_package || package_matches(text, info._name, info._display_name)) {
                        result.push_back(pkg.second);
                    }
                }
            }

            return std::move(result);
        }

        std::vector<local_version> operation::query_installed_version(const std::string &text) {
            check_valid_operation();
            std::vector<local_version> result;

            bool all_version = string_ref("all").equals_ignore_case(text);

            for (auto &ver : _core->_version_dir._versions) {
                if (all_version || version_matches(text, ver._name)) {
                    result.push_back(ver);
                }
            }

            return std::move(result);
        }

        bool operation::package_installed(const source_package &pkg) {
            auto &&locals = query_installed_package(pkg._owner_version,
                pkg._package._name);
            for (auto &p : locals) {
                if (p._info._name == pkg._package._name
                    && p._info._full_name == pkg._package._full_name
                    && p._info._version == pkg._package._version) {
                    return true;
                }
            }
            return false;
        }

        std::vector<std::string> operation::query_sources() {
            std::vector<std::string> urls;
            for (auto &s : _core->_source_dir._sources) {
                urls.push_back(s._base_url);
            }
            return std::move(urls);
        }
    }
}
