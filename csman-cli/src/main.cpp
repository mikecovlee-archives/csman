//
// Created by kiva on 2020/2/6.
//
#include <csman/cli/progress.hpp>
#include <csman/core/core.hpp>
#include <csman/core/ops.hpp>

using namespace csman::cli;
using namespace csman::core;

int main(int argc, const char **argv) {
    csman_core man("/home/kiva/csman-home");
    man.load();

    man.set_config("platform", "Linux_GCC_AMD64");
    man.unset_current_version();

    class install_latest : public operation, public mpp::event_emitter {
    public:
        void perform() override {
            auto &&result = query_version("all");
            install_version(*this, result[0]);
        }
    };

    progress_bar bar;

    install_latest op;

    op.on("ip-error", [&](const std::string &reason) {
        bar.stop(false);
    });

    op.on("ip-ok", [&]() {
        bar.tick(100);
        bar.stop(true);
    });

    op.on("ip-progress", [&](int progress, const std::string &info) {
        bar.message(info);
        bar.tick(progress);
    });

    op.on("ip-net-progress", [](int progress) {
    });

    bar.start();
    man.perform(op);

    return 0;
}
