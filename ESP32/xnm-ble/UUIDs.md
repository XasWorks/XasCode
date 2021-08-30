
# XNM BLE UUID definitions

This document shall describe and outline the UUIDs
used and defined by the XNM BLE system.
It will give a short overview of the different services and their according characteristics, as well as the data type for each characteristic.

## UUID Form

All XNM UUIDs are of the form `xxxxxxxx-2049-7fa3-0b4d-aa9e10786e6d`.
This UUID was generated with a online random UUID generator, though it may be noted that the last three bytes spell 'xnm' in ASCII :>

Service numbers will all be of the form `xxxx0000-POSTFIX`. The appended zeroes are meant to fit the UUIDs of service-specific characteristics.

As such, the UUID format for characteristics is of `ssssxxxx-POSTFIX`, where `ssss` shall match the Service's UUID

## Service types

The following is a list of services defined by, and used in, the XNM library code. Only the first two bytes, matching the UUID format specified above, will be given.

### `0f00` Debug service
This service includes a number of characteristics that specific various debugging interfaces and channels that shall aid in the development and troubleshooting of ESP32 projects. It may be left out for production runs, and is meant as developer tool rather than user interface.

The following characteristics may be included in the debug service, with the note that they may be left out depending on settings:
- `0f00 0001` Debug Stream. This characteristic will stream the ESP32 log messages (ESP_LOGx). Each message will be null-terminated, and may be split across multiple BLE packets depending on MTU size. A BLE-Terminal app shall suffice to read out the messages.
- `0f00 0002` System info JSON. This characteristic will return a JSON string when read, containing system information. This may include:
  - Free heap memory `heap` (in bytes)
  - Network status/ping `ping` (in ms round-trip-time, may be ping to/from MQTT broker)
  - WiFi RSSI/status `rssi`
  - Battery level `bat` (in % full)

### `5050` XNM property point interface
This Service shall provide an interface to the internal "PropertyPoint" configuration. This system was designed to allow multi-connection reconfiguration of the ESP from sources such as HTTP, Websockets, MQTT and BLE simultaneously. It is a lightweight, JSON-Based protocol, and is thusly well suited for a variety of generic tasks.

For further documentation, see here (*Note: Add a link and describe PropP*).

The following characteristic is part of the PropertyPoint interface:

- `5050 0001` Property Point characteristic. This characteristic streams update data to the attached BLE device in the form of JSON packets. Each packet is null-terminated, and may be spread across multiple BLE notifications to stay within BLE MTU size. The user may also write to this characteristic to send `set` commands (with longer packets again being split and null-terminated), allowing full PropP access through BLE.