//
// Created by kiva on 2020/2/5.
//

#include <csman/core/core.hpp>

int main() {
    using namespace csman::core;

    csman_core man("/home/kiva/csman-home");
    man.load();

    mpp::event_emitter ev;
    ev.on("as-progress", [](int progress) {
        printf(":: Updating source, %d%%\n", progress);
    });
    ev.on("as-error",[](const std::string &reason) {
        printf(":: Failed\n%s\n", reason.c_str());
    });
    ev.on("as-ok", [&]() {
        printf(":: OK\n");
        man.store();
    });

    man.add_source(ev, "http://mirrors.covariant.cn/csman");
    return 0;
}
