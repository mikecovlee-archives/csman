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
        }

        void operation::remove_version(mpp::event_emitter &ev, local_version &version) {

        }

        void operation::install_version(mpp::event_emitter &ev, source_version &version) {

        }

        void operation::remove_package(mpp::event_emitter &ev, local_package &pkg) {

        }

        void operation::install_package(mpp::event_emitter &ev, source_package &pkg) {

        }
    }
}
