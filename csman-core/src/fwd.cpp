//
// Created by kiva on 2020/2/5.
//

#include <csman/core/fwd.hpp>

namespace csman {
    namespace core {
        void throw_ex(const std::string &what) {
            mpp::throw_ex<source_error>(what);
        }
    }
}
