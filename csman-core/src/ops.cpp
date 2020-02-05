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

    int mix_match_rate(int name, int full_name, int display_name) {
        static constexpr int name_weight = 1;
        static constexpr int full_name_weight = 2;
        static constexpr int display_name_weight = 4;
        return name * name_weight
               + full_name * full_name_weight
               + display_name * display_name_weight;
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

        std::vector<query_result> operation::query_package(const std::string &text) {
            check_valid_operation();
            return {};
        }
    }
}
