//
// Created by kiva on 2020/2/5.
//
#pragma once

#include <mozart++/event>
#include <csman/core/fwd.hpp>
#include <string>

namespace csman {
    namespace core {
        class download_task_base : public mpp::event_emitter {
        private:
            std::string _url;
            bool _resume = false;
            long _timeout = 10;
            long _start_pos = 0;

        public:
            download_task_base() = default;

            ~download_task_base() override = default;

        public:
            void set_url(const std::string &url) {
                this->_url = url;
            }

            void resume_from_last(long position) {
                this->_resume = true;
                this->_start_pos = position;
            }

            void set_timeout(long timeout) {
                this->_timeout = timeout;
            }

            void perform();
        };

        /**
         * Events:
         *     start();
         *     progress(int progress);
         *     write(T *buffer, const char *data, size_t size, size_t nmemb, size_t *wrote);
         *     ok(T *buffer);
         *     error(const std::string &reason);
         *     end();
         *
         * @tparam T buffer type
         */
        template <typename T>
        class download_task : public download_task_base {
        private:
            T _buffer;

            void init_callbacks() {
                this->on("internal-ok", [this]() {
                    this->emit("ok", &_buffer);
                });

                this->on("internal-write", [this](const char *data, size_t size,
                                                  size_t nmemb, size_t *wrote) {
                    this->emit("write", &_buffer, data, size, nmemb, wrote);
                });

                this->on("internal-progress", [this](double total, double wrote) {
                    this->emit("progress", static_cast<int>(wrote / total * 100));
                });
            }

        public:
            download_task() {
                init_callbacks();
            }

            explicit download_task(const std::string &url) {
                set_url(url);
                init_callbacks();
            }

            ~download_task() override = default;

            download_task(download_task &&) = delete;

            download_task(const download_task &) = delete;

            download_task &operator=(download_task &&) = delete;

            download_task &operator=(const download_task &) = delete;
        };

        struct network final {
            /**
             * Event:
             *     net-progress(int progress)
             */
            static bool get_url_text(string_ref url, std::string &result,
                                     mpp::event_emitter *ev = nullptr);
        };
    }
}
