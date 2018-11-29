## What is this?
This will be a file outlining universal communcation parameters for robots, wearables, etc., intended to provide a rough outline of the health of the device in question. These parameters are intended to be sent by the device via whatever channel is available; as such, only simple datatypes and identificators should be used. Exceptions can be made for optional parameters.

## Vitality parameters
These parameters will determine whether or not a device is functional and, to some extent, what part of it has entered a critical state. They will almost always be applicable to every system.

- General Operation Status
  - Type: int8
  - Num. ID: 0xF00
  - Str. ID: "Status"
  - Interpretation: Negative values signal fatal errors, >=0 nominal (states like OK, SLEEP, LOADING)
- Power Status
  - Type: [int8, int16][]
  - Num. ID: 0xF01
  - Str. ID: "Power"
  - Interpretation: First byte signals charge percent of power source (100=FULL/OK, >100 OVERVOLT, <0 UNDERVOLT), the following 16-bit value should signal the voltage level of the power source (in 10mV).
