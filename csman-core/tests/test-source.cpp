//
// Created by kiva on 2020/2/5.
//

#include <csman/core/source.hpp>

int main() {
    using namespace csman::core;

    source_config config("http://mirrors.covariant.cn/csman");
    config.parse();

    return 0;
}
