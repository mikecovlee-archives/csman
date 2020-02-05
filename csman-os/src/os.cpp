//
// Created by kiva on 2020/2/5.
//

#include <csman/os/os.hpp>
#include "unix.hpp"
#include "win32.hpp"

namespace csman {
    namespace os {
        OS *OS::current() {
#ifdef _WIN32
            static os_impl_win32 os;
#else
            static os_impl_unix os;
#endif
            return &os;
        }
    }
}
