

#include <xasin/xai2c/LT303ALS.h>
#include <MasterAction.h>

#include <esp_log.h>

namespace Xasin {
namespace I2C{

LT303ALS::LT303ALS(uint8_t address) : addr(address), 
    last_read_data({0, 0}),  data_read_timestamp(0) {
}

esp_err_t LT303ALS::init() {
    auto i2c = XaI2C::MasterAction(addr);

    als_control_t ctrl = {};
    ctrl.als_mode = 1;
    ctrl.als_gain = 6;

    i2c.write(ALS_CONTR, &ctrl, 1);
    
    auto ret = i2c.execute();

    if(ret != ESP_OK)
        ESP_LOGE("LT303ALS", "Chip did not init!");

    return ret;
}

als_data_t LT303ALS::get_brightness() {
    if(xTaskGetTickCount() - data_read_timestamp < 500/portTICK_PERIOD_MS)
        return last_read_data;

    auto i2c = XaI2C::MasterAction(addr);

    i2c.read(ALS_DATA_START, &last_read_data, sizeof(last_read_data));
    
    if(i2c.execute() == ESP_OK)
        data_read_timestamp = xTaskGetTickCount();

    return last_read_data;
}

}
}