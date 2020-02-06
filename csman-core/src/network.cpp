//
// Created by kiva on 2020/1/31.
//

#include <csman/core/network.hpp>
#include <mozart++/event>
#include <curl/curl.h>

namespace {
    using csman::core::download_task_base;

    size_t callback_scan_length(void *data, size_t size, size_t nmemb, void *userdata) {
        long len = 0;
        if (sscanf(reinterpret_cast<const char *>(data),
            "Content-Length: %ld\n", &len) > 0) {
            *((long *) userdata) = len;
        }

        return size * nmemb;
    }

    size_t callback_write(void *ptr, size_t size, size_t nmemb, void *userdata) {
        auto *d = reinterpret_cast<download_task_base *>(userdata);
        size_t wrote = 0;
        d->emit("internal-write", reinterpret_cast<const char *>(ptr),
            size, nmemb, &wrote);
        return wrote;
    }

    int callback_progress(void *userdata, double total, double wrote, double utotal, double uwrote) {
        auto *d = reinterpret_cast<download_task_base *>(userdata);
        if (total == 0) {
            return 0;
        }
        d->emit("internal-progress", total, wrote);
        return 0;
    }
}

namespace csman {
    namespace core {
        bool network::get_url_text(string_ref url, std::string &result, mpp::event_emitter *ev) {
            bool ok = false;
            download_task<std::string> task(url.str());

            task.on("write", [](std::string *buffer,
                                const char *data, size_t size,
                                size_t nmemb, size_t *wrote) {
                buffer->append(data);
                *wrote = size * nmemb;
            });

            task.on("ok", [&result, &ok](std::string *content) {
                ok = true;
                result = *content;
            });

            task.on("error", [&result](const std::string &reason) {
                result = reason;
            });

            if (ev != nullptr) {
                task.on("progress", [ev](int progress) {
                    ev->emit("net-progress", progress);
                });
            }

            task.perform();
            return ok;
        }

        void download_task_base::perform() {
            if (_url.empty()) {
                this->emit("error", std::string("url is empty"));
                return;
            }

            CURL *curl = curl_easy_init();
            if (!curl) {
                this->emit("error", std::string("unable to init curl"));
                return;
            }

            // Common
            curl_easy_setopt(curl, CURLOPT_URL, _url.c_str());
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, _timeout);

            // Don't check https certificate
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

            // Resume from break point
            long file_length = 0;
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &file_length);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, callback_scan_length);
            curl_easy_setopt(curl, CURLOPT_RESUME_FROM_LARGE, _resume ? _start_pos : 0);

            // Write function
            curl_easy_setopt(curl, CURLOPT_WRITEDATA,
                reinterpret_cast<download_task_base *>(this));
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback_write);

            // Progress bar
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
            curl_easy_setopt(curl, CURLOPT_PROGRESSDATA,
                reinterpret_cast<download_task_base *>(this));
            curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, callback_progress);

            // Perform request
            this->emit("start");

            CURLcode ret;
            if ((ret = curl_easy_perform(curl)) != 0) {
                this->emit("error", std::string(curl_easy_strerror(ret)));
            } else {
                this->emit("internal-ok");
                this->emit("end");
            }

            curl_easy_cleanup(curl);
        }
    }
}
