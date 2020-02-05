//
// Created by kiva on 2020/2/5.
//

#include <csman/core/network.hpp>

int main() {
    using namespace csman::core;

    std::string json;
    if (network::get_url_text("http://mirrors.covariant.cn/csman/csman.json", json)) {
        printf("json:\n%s\n", json.c_str());
    } else {
        printf("failed: %s\n", json.c_str());
    }
}
