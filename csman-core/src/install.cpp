//
// Created by kiva on 2020/2/5.
//

#include <csman/os/os.hpp>
#include <csman/core/ops.hpp>

namespace csman {
    namespace core {
        using namespace csman::os;

        void operation::checkout_version(mpp::event_emitter &ev, local_version &version) {
            // first of all, make sure that the version exists.
            // we only check the versions/<version>'s existence
            // and we don't care what's actually inside.
            if (!OS::current()->directory_exists(version._path)) {
                ev.emit("co-error", std::string(
                    "version " + version._name + " doesn't exists: "
                    + OS::current()->error()));
                return;
            }

            // secondly, create symbolic link
            // from versions/current to versions/<version>
            std::string current_dir =
                _core->_version_dir._path + mpp::path_separator + "current";
            if (!OS::current()->ln(version._path, current_dir)) {
                ev.emit("co-error", std::string(
                    "failed to create symlink from " + version._path
                    + " to " + current_dir + ": " + OS::current()->error()));
                return;
            }

            // finally, update config
            _core->set_current_version(version._name);
            ev.emit("co-ok");
        }

        void operation::remove_version(mpp::event_emitter &ev, local_version &version) {
            // first of all, unset current version
            // if we are removing it
            ev.emit("rv-progress", 10);
            auto &&cv = optional_current_version();
            if (cv.has_value() && version._name == cv.get()) {
                std::string current_dir =
                    _core->_version_dir._path + mpp::path_separator + "current";
                ev.emit("rv-progress", 40);
                if (!OS::current()->unlink(current_dir)) {
                    ev.emit("rv-error", std::string(
                        "failed to remove current version " + version._name
                        + ": unable to remove symlink: "
                        + OS::current()->error()));
                    return;
                }
                _core->unset_current_version();
            }

            // secondly, remove everything under version.
            // DO NOT check errors here, because we have removed
            // current version config globally,
            // just proceed when error occurs.
            ev.emit("rv-progress", 70);
            OS::current()->rm_rf(version._path);

            // and... that's all
            ev.emit("rv-progress", 100);
            ev.emit("rv-ok");
        }

        void operation::install_version(mpp::event_emitter &ev, source_version &version) {

        }

        void operation::remove_package(mpp::event_emitter &ev, local_package &pkg) {

        }

        void operation::install_package(mpp::event_emitter &ev, source_package &pkg) {

        }
    }
}
