//
// Created by kiva on 2020/2/6.
//
#include <csman/cli/progress.hpp>
#include <chrono>

using namespace csman::cli;

int main(int argc, const char **argv) {
    progress_bar bar;
    bar.set_width(50);
    bar.start();

    for (int i = 0; i <= 100; ++i) {
        bar.tick(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return 0;
}
