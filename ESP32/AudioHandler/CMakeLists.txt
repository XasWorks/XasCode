
idf_component_register(SRCS "AudioTX.cpp" "Source.cpp" "ByteCassette.cpp" "OpusCassette.cpp" "TXStream.cpp"
							"RX.cpp"
                       INCLUDE_DIRS "include"
                       REQUIRES MQTT_SubHandler esp-libopus)
