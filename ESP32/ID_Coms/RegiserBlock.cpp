/*
 * RegisterBlock.cpp
 *
 *  Created on: 9 Jan 2019
 *      Author: xasin
 */

#include "RegisterBlock.h"

#include <cstring>

namespace Xasin {
namespace Communication {

SlaveChannel::SlaveChannel(RegisterBlock &mainRegister)
	: mainRegister(mainRegister) {
}

void SlaveChannel::send_update(uint16_t ID, Data_Packet data) {}

ComRegister::ComRegister(uint16_t ID, RegisterBlock &registers, Data_Packet dataLoc, bool write_allowed)
	: registerBlock(registers),
	  dataLocation(nullptr), dataLength(0),
	  write_cb(nullptr),
	  ID(ID),
	  write_allowed(write_allowed) {}

void ComRegister::update(Data_Packet data) {
	registerBlock.send_update(this->ID, data);
}
void ComRegister::update() {
	registerBlock.send_update(this->ID, {dataLength, dataLocation});
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
}

} /* namespace Communication */
} /* namespace Xasin */
