//
// Created by kiva on 2020/2/5.
//

#include <csman/core/core.hpp>
#include <csman/core/ops.hpp>

int main() {
    using namespace csman::core;

    csman_core man("/home/kiva/csman-home");
    man.load();

    man.set_config("platform", "Linux_GCC_AMD64");

    class query : public operation {
    public:
        void perform() override {
            std::string query_text = "official";
            auto &&result = query_package(query_text);
            printf("query(%s):\n", query_text.c_str());

            for (auto &r : result) {
                printf("  %4d: [%s][%s][%s]\n",
                    r.match_rate,
                    r._package._name.c_str(),
                    r._package._full_name.c_str(),
                    r._package._display_name.c_str());
            }
        }
    };

    query op;
    man.perform(op);

    return 0;
}
