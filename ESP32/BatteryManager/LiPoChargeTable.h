/*
 * LiPoChargeTable.h
 *
 *  Created on: 9 Jan 2019
 *      Author: xasin
 */

#ifndef XASLIBS_BATTERYMANAGER_LIPOCHARGETABLE_H_
#define XASLIBS_BATTERYMANAGER_LIPOCHARGETABLE_H_

#include "stdint.h"
#include <vector>

struct CHARGE_POINT {
	uint8_t  percentage;
	uint16_t voltage;
};

// TODO - replace this with a real, measured curve!

static const std::vector<CHARGE_POINT> discharge_curve {
	{100, 	4200},
	{90,	4080},
	{80,	3930},
	{70,	3860},
	{60,	3800},
	{50,	3760},
	{40,	3720},
	{30,	3700},
	{20,	3660},
	{10,	3620},
	{7,		3600},
	{5,		3410},
	{0,		3000}
};

#endif /* XASLIBS_BATTERYMANAGER_LIPOCHARGETABLE_H_ */
