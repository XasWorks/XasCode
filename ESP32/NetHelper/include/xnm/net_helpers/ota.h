

#pragma once

#include <stdint.h>

namespace XNM {
namespace NetHelpers {

namespace OTA {

#pragma pack(1)
    union ota_version_t {
        uint32_t u32;

        struct {
            uint8_t release_ver:1;
            uint8_t branch_id:7;
            uint16_t minor_vers:13;
            uint8_t month_vers:4;
            uint8_t year_vers:7;
        };

        ota_version_t();
        ota_version_t(uint32_t int_version);
    };
#pragma pack(0)

    enum ota_state_t {
        UNKNOWN,
        UP_TO_DATE,
		UPDATE_AVAILABLE,
        DOWNLOADING,
        UNVERIFIED,
    };

    void housekeep_tick();
    void init();

    void cancel_rollback();
    void force_rollback();

    const char * get_branch_name();

    void print_version(char * buffer, size_t b_size, ota_version_t vers);
    void report_version();

    ota_state_t get_state();

    ota_version_t get_local_version();
    ota_version_t get_remote_version();

    void set_upstream_version(ota_version_t up_version);
}

}
}