//
// Created by kiva on 2020/2/6.
//
#pragma once

#include <memory>
#include <atomic>
#include <thread>

namespace csman {
    namespace cli {
        class progress_bar {
        private:
            std::unique_ptr<std::thread> _thread{nullptr};
            std::atomic_bool _draw{true};
            std::atomic_int _progress{0};
            int _width = 0;

        public:
            progress_bar() = default;

            ~progress_bar();

            void start();

            void stop() {
                this->_draw = false;
            }

            void tick(int progress) {
                this->_progress = progress;
            }

            void set_width(int w) {
                this->_width = w;
            }
        };
    }
}
