

#pragma once

#include <stdint.h>

namespace XNM {
namespace NetHelpers {

namespace OTA {
    enum ota_state_t {
        UNKNOWN,
        UP_TO_DATE,
		UPDATE_AVAILABLE,
        DOWNLOADING,
        REBOOT_NEEDED,
        UNVERIFIED,
    };

    void init();
    void cancel_rollback();

    ota_state_t get_state();

    int32_t get_local_version();
    void set_upstream_version(int32_t up_version);
}

}
}