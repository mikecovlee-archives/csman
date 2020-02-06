//
// Created by kiva on 2020/1/31.
//

#include <chrono>
#include <csman/os/os.hpp>
#include <csman/cli/progress.hpp>

namespace csman {
    namespace cli {
        using namespace csman::os;

        void progress_bar::start() {
            _thread = std::make_unique<std::thread>(
                [](progress_bar &bar) {
                    static constexpr char C[] = {
                        '/', '-', '\\', '|',
                    };

                    int width = bar._width;
                    size_t index = 0;
                    auto *os = OS::current();

                    while (bar._draw) {
                        int progress = bar._progress;
                        if (index >= sizeof(C)) {
                            index = 0;
                        }

                        if (progress > 100) {
                            bar._progress = 0;
                            progress = 0;
                            os->rewind_cursor();
                            printf("  ✔︎\n");
                        }

                        printf("  %c  %3d%% ", C[index++ % sizeof(C)], progress);

                        int complete = width * progress / 100;
                        int empty = width - complete;

                        // progress bar start mark
                        putchar('[');

                        // finished part
                        for (int i = 0; i < complete - 1; ++i) {
                            putchar('=');
                        }

                        // processing part
                        putchar('>');

                        // future part
                        for (int i = 0; i < empty; ++i) {
                            putchar(' ');
                        }

                        // progress bar end mark
                        putchar(']');

                        // workaround for windows terminal:
                        // fix duplicated tailing progress bar end mark
                        putchar(' ');

                        // display it!
                        os->rewind_cursor();
                        fflush(stdout);
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }

                    os->rewind_cursor();
                    printf("  %s\n", bar._status ? "✔︎" : "🔨");
                    fflush(stdout);
                },
                std::ref(*this));
        }

        progress_bar::~progress_bar() {
            stop(true);
        }

        void progress_bar::stop(bool status) {
            this->_status = status;
            this->_draw = false;
            if (_thread != nullptr && _thread->joinable()) {
                _thread->join();
            }
        }
    }
}

