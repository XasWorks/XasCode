/*
 * MasterAction.h
 *
 *  Created on: 24 Nov 2018
 *      Author: xasin
 */

#ifndef COMPONENTS_XAI2C_MASTERACTION_H_
#define COMPONENTS_XAI2C_MASTERACTION_H_

#include "driver/i2c.h"

namespace XaI2C {

class MasterAction {
private:
	bool executed;

	i2c_cmd_handle_t cmd;

	const uint8_t slave;

	void send_start(bool rw);

public:
	static void init(gpio_num_t sda, gpio_num_t scl, i2c_port_t port = I2C_NUM_0);

	MasterAction(uint8_t sla_addr);
	virtual ~MasterAction();

	void write(uint8_t cCode, void *data = nullptr, size_t length = 0);

	void read(uint8_t cCode, void *data, size_t length);
	void read(void *data, size_t length);

	void execute(i2c_port_t port = I2C_NUM_0);
};

} /* namespace XaI2C */

#endif /* COMPONENTS_XAI2C_MASTERACTION_H_ */
