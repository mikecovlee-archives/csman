//
// Created by kiva on 2020/2/6.
//
#include <csman/cli/progress.hpp>
#include <csman/core/core.hpp>
#include <csman/core/ops.hpp>
#include <utility>

using namespace csman::cli;
using namespace csman::core;

int do_update(csman_core &man) {
    // TODO
    return 0;
}

int do_list(csman_core &man) {
    class list : public operation, public mpp::event_emitter {
    public:
        void perform() override {
            auto &&result = query_installed_package("all");
            if (!result.empty()) {
                for (auto &p : result) {
                    printf("%s(%s) from runtime version %s\n",
                        p._info._name.c_str(),
                        p._info._version.c_str(),
                        p._owner_version.c_str());
                }
            }
        }
    };
    list op;
    man.perform(op);
    return 0;
}

int do_list_version(csman_core &man) {
    class list : public operation, public mpp::event_emitter {
    public:
        void perform() override {
            auto &&result = query_installed_version("all");
            if (!result.empty()) {
                for (auto &p : result) {
                    printf("%s\n", p._name.c_str());
                }
            }
        }
    };
    list op;
    man.perform(op);
    return 0;
}

int do_add_source(csman_core &man, mpp::string_ref url) {
    progress_bar bar;
    mpp::event_emitter ev;
    bool error = false;

    ev.on("as-progress", [&](int progress) {
        bar.tick(progress);
    });
    ev.on("as-error", [&](const std::string &reason) {
        bar.message(reason);
        bar.stop(false);
        error = true;
    });
    ev.on("as-ok", [&]() {
        bar.stop(true);
        man.store();
    });

    printf("Adding source %s\n", url.str().c_str());
    bar.start();
    man.add_source(ev, url);
    return error ? 1 : 0;
}

int do_install(csman_core &man, mpp::string_ref package) {
    class install : public operation, public mpp::event_emitter {
    private:
        std::string _package;
    public:
        explicit install(std::string package)
            : _package(std::move(package)) {}

        void perform() override {
            auto &&result = query_package(_package);
            if (result.empty()) {
                printf("Cannot locate package %s\n", _package.c_str());
                return;
            }
            if (result.size() > 1) {
                printf("Too many candidates, please specify one:\n");
                for (auto &p : result) {
                    printf("   %s(%s) from runtime version %s\n",
                        p._package._name.c_str(),
                        p._package._version.c_str(),
                        p._owner_version.c_str());
                }
                return;
            }
            emit("ip-start");
            install_package(*this, result[0]);
        }
    };

    bool error = false;
    progress_bar bar;
    install op(package);

    op.on("ip-start", [&]() {
        bar.start();
    });

    op.on("ip-error", [&](const std::string &reason) {
        bar.message(reason);
        bar.stop(false);
        error = true;
    });

    op.on("ip-ok", [&]() {
        bar.tick(100);
        bar.stop(true);
    });

    op.on("ip-progress", [&](int progress, const std::string &info) {
        bar.message(info);
    });

    op.on("ip-net-progress", [&](int progress) {
        bar.tick(progress);
    });

    man.perform(op);
    return error ? 1 : 0;
}

int do_remove(csman_core &man, mpp::string_ref package) {
    class remove : public operation, public mpp::event_emitter {
    private:
        std::string _package;
    public:
        explicit remove(std::string package)
            : _package(std::move(package)) {}

        void perform() override {
            auto &&result = query_installed_package(_package);
            if (result.empty()) {
                printf("Package not installed on current version: %s\n", _package.c_str());
                return;
            }
            if (result.size() > 1) {
                printf("Too many candidates, please specify one:\n");
                for (auto &p : result) {
                    printf("   %s(%s)\n", p._info._name.c_str(),
                        p._info._version.c_str());
                }
                return;
            }
            emit("rp-start");
            remove_package(*this, result[0]);
        }
    };

    bool error = false;
    progress_bar bar;
    remove op(package);

    op.on("rp-start", [&]() {
        bar.start();
    });

    op.on("rp-error", [&](const std::string &reason) {
        bar.message(reason);
        bar.stop(false);
        error = true;
    });

    op.on("rp-ok", [&]() {
        bar.tick(100);
        bar.stop(true);
    });

    op.on("ip-progress", [&](int progress) {
        bar.tick(progress);
    });

    man.perform(op);
    return error ? 1 : 0;
}


int do_checkout(csman_core &man, mpp::string_ref version) {
    class checkout : public operation, public mpp::event_emitter {
    private:
        std::string _version;
    public:
        explicit checkout(std::string package)
            : _version(std::move(package)) {}

        void perform() override {
            auto &&result = query_installed_version(_version);
            if (result.empty()) {
                printf("Version not installed %s\n", _version.c_str());
                return;
            }
            if (result.size() > 1) {
                printf("Too many candidates, please specify one:\n");
                for (auto &p : result) {
                    printf("   %s\n", p._name.c_str());
                }
                return;
            }
            checkout_version(*this, result[0]);
        }
    };

    bool error = false;
    checkout op(version);

    op.on("ip-error", [&](const std::string &reason) {
        fprintf(stderr, "%s\n", reason.c_str());
        error = true;
    });

    op.on("ip-ok", [&]() {
        printf("Version checked-out to %s\n", version.str().c_str());
    });

    man.perform(op);
    return error ? 1 : 0;
}

int do_config(csman_core &man, int argc, const char **argv) {
    --argc;
    ++argv;

    if (argc == 0) {
        fprintf(stderr, "Please see README.md\n");
        return 1;
    }

    for (; argc != 0 && *argv; --argc, ++argv) {
        if (!strcmp(argv[0], "set")) {
            if (argc != 3) {
                fprintf(stderr, "Usage: csman config set <key> <value>\n");
                return 1;
            }
            man.set_config(argv[1], argv[2]);
            return 0;
        }

        if (!strcmp(argv[0], "get")) {
            if (argc != 2) {
                fprintf(stderr, "Usage: csman config get <key>\n");
                return 1;
            }
            printf("%s\n", man.get_config(argv[1]).c_str());
            return 0;
        }

        if (!strcmp(argv[0], "unset")) {
            if (argc != 2) {
                fprintf(stderr, "Usage: csman config unset <key>\n");
                return 1;
            }
            man.unset_config(argv[1]);
            return 0;
        }
    }

    fprintf(stderr, "Please see README.md\n");
    return 1;
}


int main(int argc, const char **argv) {
    --argc;
    ++argv;

    csman_core man("/home/kiva/csman-home");
    man.load();

    if (argc == 0) {
        fprintf(stderr, "Please see README.md\n");
        return 1;
    }

    try {
        for (; argc != 0 && *argv; --argc, ++argv) {
            if (!strcmp(argv[0], "update")) {
                return do_update(man);
            }

            if (!strcmp(argv[0], "add-source")) {
                if (argc > 1) {
                    return do_add_source(man, argv[1]);
                } else {
                    fprintf(stderr, "Usage: csman add-source <url>\n");
                    return 1;
                }
            }

            if (!strcmp(argv[0], "install")) {
                if (argc > 1) {
                    return do_install(man, argv[1]);
                } else {
                    fprintf(stderr, "Usage: csman install <package>\n");
                    return 1;
                }
            }

            if (!strcmp(argv[0], "checkout")) {
                if (argc > 1) {
                    return do_checkout(man, argv[1]);
                } else {
                    fprintf(stderr, "Usage: csman checkout <version>\n");
                    return 1;
                }
            }

            if (!strcmp(argv[0], "list")) {
                return do_list(man);
            }

            if (!strcmp(argv[0], "list-version")) {
                return do_list_version(man);
            }

            if (!strcmp(argv[0], "remove") || !strcmp(argv[0], "uninstall")) {
                if (argc > 1) {
                    return do_remove(man, argv[1]);
                } else {
                    fprintf(stderr, "Usage: csman %s <package>\n", argv[0]);
                    return 1;
                }
            }

            if (!strcmp(argv[0], "config")) {
                return do_config(man, argc, argv);
            }
        }

    } catch (source_error &error) {
        printf("%s\n", error.what());
        return 0;
    }

    fprintf(stderr, "Please see README.md\n");
    return 1;
}
