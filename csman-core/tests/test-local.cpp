//
// Created by kiva on 2020/2/5.
//

#include <csman/core/local.hpp>

int main() {
    using namespace csman::core;

    csman_core man("/home/kiva/csman-home");
    man.load();
    man.add_source("http://mirrors.covariant.cn/csman");
    man.store();

    return 0;
}
