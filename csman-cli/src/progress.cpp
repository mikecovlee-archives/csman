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
            _draw = true;
            _thread = std::make_unique<std::thread>(&progress_bar::draw, this);
        }

        progress_bar::~progress_bar() {
            stop(_status);
        }

        void progress_bar::stop(bool status) {
            this->_status = status;
            this->_draw = false;
            if (_thread != nullptr && _thread->joinable()) {
                _thread->join();
            }
        }

        void progress_bar::draw(progress_bar *bar) {
            static constexpr char C[] = {
                '/', '-', '\\', '|',
            };

            auto *os = OS::current();
            const int terminal_width = os->terminal_width();
            size_t index = 0;

            while (true) {
                if (!bar->_draw) {
                    if (!bar->_ticked) {
                        break;
                    } else {
                        bar->_ticked = false;
                    }
                }

                // reset current line and print message
                os->rewind_cursor();
                for (int i = 0; i < terminal_width; ++i) {
                    putchar(' ');
                }
                os->rewind_cursor();

                if (bar->_message_changed) {
                    printf("%s\n", static_cast<const char *>(bar->_message));
                    // notify processed
                    bar->_message_changed = false;
                }

                int progress = bar->_progress;

                int used = printf("  %c  %3d%% ", C[index++ % sizeof(C)], progress);
                int remaining = terminal_width - used - 4;

                int complete = remaining * progress / 100;
                int empty = remaining - complete;

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

                // display it!
                os->rewind_cursor();
                fflush(stdout);
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            os->rewind_cursor();
            printf("  %s\n", bar->_status ? "✔︎" : "🔨");
            fflush(stdout);
        }
    }
}

