
#pragma once

#include <stdint.h>
#include <esp_err.h>

#include <freertos/FreeRTOS.h>

namespace Xasin {
namespace I2C {

#pragma pack(1)
    struct als_control_t {
        uint8_t als_mode:1;
        uint8_t sw_reset:1;
        uint8_t als_gain:3;
    };

    struct als_meas_time_t {
        uint8_t als_repeat_rate:3;
        uint8_t als_integration_time:3;
    };

    struct als_data_t {
        uint16_t als_ch1;
        uint16_t als_ch0;
    };
#pragma pack(0)

class LT303ALS {
public:
    enum reg_addr_t : uint8_t {
        ALS_CONTR = 0x80,
        ALS_MEAS_RATE = 0x85,
        PART_ID = 0x86,
        MANU_ID = 0x87,
        ALS_DATA_START = 0x88,
    };

private:
    const uint8_t addr;
    als_data_t last_read_data;
    TickType_t data_read_timestamp;

public:
    LT303ALS(uint8_t address = 0x29);

    esp_err_t init();

    als_data_t get_brightness();
};

}
}