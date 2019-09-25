/*
 * MasterAction.cpp
 *
 *  Created on: 24 Nov 2018
 *      Author: xasin
 */

#include "MasterAction.h"

namespace XaI2C {

void MasterAction::init(gpio_num_t sda, gpio_num_t scl, i2c_port_t port, uint32_t speed) {
	i2c_config_t i2c_cfg = {};

	i2c_cfg.mode = I2C_MODE_MASTER;
	i2c_cfg.scl_io_num = scl;
	i2c_cfg.sda_io_num = sda;

	i2c_cfg.scl_pullup_en = GPIO_PULLUP_ENABLE;
	i2c_cfg.sda_pullup_en = GPIO_PULLUP_ENABLE;

	i2c_cfg.master.clk_speed = speed;

	i2c_param_config(port, &i2c_cfg);

	i2c_driver_install(port, I2C_MODE_MASTER, 0, 0, 0);

	i2c_set_timeout(port, 60000);
}

MasterAction::MasterAction(uint8_t slave) :
		executed(false), slave(slave) {
	cmd = i2c_cmd_link_create();
}

MasterAction::~MasterAction() {
	if(!executed)
		i2c_cmd_link_delete(cmd);
}

void MasterAction::send_start(bool rw) {
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (slave << 1) | rw, true);
}

void MasterAction::write(uint8_t cCode, void *data, size_t length) {
	send_start(false);

	i2c_master_write_byte(cmd, cCode, true);
	if(data != nullptr)
		i2c_master_write(cmd, reinterpret_cast<unsigned char *>(data), length, true);
}

void MasterAction::read(void *data, size_t length) {
	assert(data != nullptr);

	send_start(true);

	i2c_master_read(cmd, reinterpret_cast<unsigned char *>(data), length, I2C_MASTER_LAST_NACK);
}
void MasterAction::read(uint8_t cCode, void *data, size_t length) {
	write(cCode);
	read(data, length);
}

esp_err_t MasterAction::execute(i2c_port_t port) {
	assert(!executed);

	i2c_master_stop(cmd);

	executed = true;
	esp_err_t ret = i2c_master_cmd_begin(port, cmd, 1000);

	i2c_cmd_link_delete(cmd);

	return ret;
}

esp_err_t MasterAction::poke(uint8_t addr, i2c_port_t port) {
	auto i2c = MasterAction(addr);

	i2c.send_start(false);

	return i2c.execute(port);
}

} /* namespace XaI2C */
