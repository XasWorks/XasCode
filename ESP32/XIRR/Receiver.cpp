/*
 * Receiver.cpp
 *
 *  Created on: 7 Jun 2019
 *      Author: xasin
 */

#include "xasin/xirr/Receiver.h"

#define LOG_LOCAL_LEVEL ESP_LOG_INFO
#include "esp_log.h"

#define TPB ((80000000/200)/2000)

namespace Xasin {
namespace XIRR {

void ir_rx_task(void *args) {
	reinterpret_cast<Receiver*>(args)->_rx_task();
}

Receiver::Receiver(gpio_num_t pin, rmt_channel_t channel) :
	rxPin(pin), rmtChannel(channel),
	rxTaskHandle(nullptr),
	currentItem(nullptr), itemOffset(0), numItems(0) {
}

Receiver::~Receiver() {
	rmt_driver_uninstall(rmtChannel);
	gpio_reset_pin(rxPin);

	vTaskDelete(rxTaskHandle);
}

void Receiver::init() {
	gpio_reset_pin(rxPin);

	rmt_config_t cfg = {};
	rmt_rx_config_t rx_cfg = {};

	rx_cfg.filter_en = true;
	rx_cfg.filter_ticks_thresh = 200;
	rx_cfg.idle_threshold = TPB*9;

	cfg.rx_config = rx_cfg;

	cfg.rmt_mode = RMT_MODE_RX;

	cfg.clk_div = 200;
	cfg.gpio_num = rxPin;
	cfg.mem_block_num = 1;
	cfg.channel = rmtChannel;

	rmt_config(&cfg);
	rmt_driver_install(rmtChannel, 1000, 0);

	xTaskCreate(ir_rx_task, "XIRR:RX", 4096, this, 5, &rxTaskHandle);
}

bool Receiver::get_next_bit() {
	if(numItems == 0)
		return false;

	bool out = false;

	if(itemOffset < currentItem->duration0)
		out = !currentItem->level0;
	else
		out = !currentItem->level1;

	auto tDur = currentItem->duration0 + currentItem->duration1;

	if(itemOffset < currentItem->duration0
			&& (itemOffset + TPB) > currentItem->duration0) {
		itemOffset = currentItem->duration0 + 0.5*TPB;
	}
	else if((itemOffset + TPB) > tDur) {
		itemOffset = 0.5*TPB;
		currentItem++;
		numItems--;
	}
	else
		itemOffset += TPB;

	ESP_LOGV("XIRR", "Bit: %s", out ? "1" : "0");
	return out;
}

void Receiver::parse_item(rmt_item32_t *head, size_t len) {
	if(head == nullptr)
		return;
	if(len < 2)
		return;

	currentItem = head;
	itemOffset  = TPB*0.5;
	numItems   = len;

	for(uint8_t i=0; i<8; i++) {
		if(get_next_bit() != (i != 7))
			return;
	}

	ESP_LOGV("XIRR", "Start OK");

	uint8_t chksum = 0;
	std::vector<uint8_t> dataBuffer;

	while(true) {
		uint8_t byteBuf = 0;

		for(uint8_t i=0; i<8; i++)
			byteBuf |= get_next_bit() << i;

		dataBuffer.push_back(byteBuf);
		if(!get_next_bit()) {
			break;
		}

		chksum += byteBuf;
	}

	ESP_LOG_BUFFER_HEX_LEVEL("XIRR", dataBuffer.data(), dataBuffer.size(), ESP_LOG_VERBOSE);

	if(dataBuffer[dataBuffer.size()-1] == chksum && on_rx != nullptr)
		on_rx(dataBuffer.data()+1, dataBuffer.size()-2, dataBuffer[0]);
}

void Receiver::_rx_task() {
	RingbufHandle_t rx_buffer = nullptr;

	rmt_get_ringbuf_handle(rmtChannel, &rx_buffer);
	rmt_rx_start(rmtChannel, 1);

	ESP_LOGI("XIRR", "RX Task started!");

	while(true) {
		size_t dNum = 0;
		rmt_item32_t *headItem = reinterpret_cast<rmt_item32_t*>(xRingbufferReceive(rx_buffer, &dNum, portMAX_DELAY));

		ESP_LOGV("XIRR", "Data received, %d items", dNum);

		if(headItem == nullptr)
			continue;

		parse_item(headItem, dNum);

		vRingbufferReturnItem(rx_buffer, headItem);
	}
}

} /* namespace XIRR */
} /* namespace Xasin */
