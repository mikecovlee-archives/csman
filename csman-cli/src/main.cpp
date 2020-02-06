//
// Created by kiva on 2020/2/6.
//
#include <csman/cli/progress.hpp>
#include <csman/core/core.hpp>
#include <chrono>

using namespace csman::cli;
using namespace csman::core;

int main(int argc, const char **argv) {
    csman_core man("/home/kiva/csman-home");
    man.load();

    mpp::event_emitter ev;
    progress_bar bar;
    bar.set_width(50);

    ev.on("as-progress", [&](int progress) {
        bar.tick(progress);
    });
    ev.on("as-error",[](const std::string &reason) {
        printf(":: Failed\n%s\n", reason.c_str());
    });
    ev.on("as-ok", [&]() {
        bar.tick(100);
        bar.stop();
        man.store();
    });

    const char *url = "http://mirrors.covariant.cn/csman";
    printf(":: Updating source info: %s\n", url);
    bar.start();
    man.add_source(ev, url);
    return 0;
}
