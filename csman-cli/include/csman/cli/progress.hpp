//
// Created by kiva on 2020/2/6.
//
#pragma once

#include <memory>
#include <atomic>
#include <thread>
#include <cstring>

namespace csman {
    namespace cli {
        class progress_bar {
        private:
            std::unique_ptr<std::thread> _thread{nullptr};
            std::atomic_bool _draw{false};
            std::atomic_bool _status{true};
            std::atomic_bool _ticked{false};
            std::atomic_int _progress{0};

        private:
            char _message_buffer[256]{};

            std::atomic<const char *> _message{_message_buffer};
            std::atomic_bool _message_changed{false};

        private:
            static void draw(progress_bar *bar);

        public:
            progress_bar() = default;

            ~progress_bar();

            void start();

            void stop(bool status);

            void tick(int progress) {
                this->_progress = progress;
                this->_ticked = true;
            }

            void message(const std::string &msg) {
                strncpy(_message_buffer, msg.c_str(), sizeof(_message_buffer));
                this->_message_changed = true;
                while (_message_changed) {
                    // wait for completion
                }
            }
        };
    }
}
