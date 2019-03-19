/*
 * RegisterBlock.cpp
 *
 *  Created on: 9 Jan 2019
 *      Author: xasin
 */

#include "RegisterBlock.h"

#include <stdio.h>
#include <cstring>

#include "esp_log.h"

namespace Xasin {
namespace Communication {

SlaveChannel::SlaveChannel(RegisterBlock &mainRegister)
	: mainRegister(mainRegister) {

	mainRegister.channels.push_back(this);
}

void SlaveChannel::send_update(uint16_t ID, Data_Packet data, bool retained) {
}

ComRegister::ComRegister(uint16_t ID, RegisterBlock &registers, void *dataLoc, size_t dataLen, bool write_allowed)
	: registerBlock(registers),
	  dataLocation(dataLoc), dataLength(dataLen),
	  write_cb(nullptr),
	  ID(ID),
	  write_allowed(write_allowed),
	  retained(false) {

	ESP_LOGI(IDCOMM_TAG, "Opened register with ID: %0X", ID);
	registerBlock.comRegisters[ID] = this;
}

void ComRegister::update(Data_Packet data) {
	registerBlock.send_update(this->ID, data, this->retained);
}
void ComRegister::update() {
	registerBlock.send_update(this->ID, {dataLength, dataLocation}, this->retained);
}
void ComRegister::update(const std::string &data) {
	update({data.length(), data.data()});
}

RegisterBlock::RegisterBlock()
	: channels(), comRegisters() {}

void RegisterBlock::write_register(uint16_t ID, const Data_Packet data) {
	if(comRegisters.count(ID) > 0) {
		ComRegister &r = *comRegisters[ID];
		if(r.write_cb != nullptr)
			r.write_cb(data);

		if(r.write_allowed && r.dataLength == data.length) {
			memcpy(r.dataLocation, data.data, data.length);
			this->send_update(ID, data);
		}
	}
	else
		ESP_LOGW(IDCOMM_TAG, "No register found for %d", ID);
}

void RegisterBlock::send_update(uint16_t ID, Data_Packet data, bool retained) {
	for(auto c : channels) {
		c->send_update(ID, data, retained);
	}
}

} /* namespace Communication */
} /* namespace Xasin */
