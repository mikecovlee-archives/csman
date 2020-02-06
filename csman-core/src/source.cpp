//
// Created by kiva on 2020/1/31.
//

#include <csman/core/source.hpp>
#include <csman/core/parser.hpp>
#include <csman/core/network.hpp>

namespace csman {
    namespace core {
        void source_updater::update() {
            try {
                source_root_info info;

                std::string json;
                std::string root_info_url = _source_url + "/" + INFO_FILE;

                if (!network::get_url_text(root_info_url, json, this)) {
                    throw_ex("Failed to get file: " + root_info_url
                             + ": " + json);
                }

                // Inherit base url from parent.
                // For root config, the parent is user.
                info._base_url = _source_url;
                parse_root(info, json);

                this->emit("su-ok", info);
            } catch (source_error &err) {
                this->emit("su-error", std::string(err.what()));
            }
        }
    }
}
