//
// Created by kiva on 2020/1/31.
//

#include <csman/core/source.hpp>
#include <csman/core/parser.hpp>

namespace csman {
    namespace core {
        void source_config::parse() {
            parse_root_url(_info, _source_url);
        }
    }
}
