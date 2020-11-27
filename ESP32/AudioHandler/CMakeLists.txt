
idf_component_register(SRCS "AudioTX.cpp" "Source.cpp" "ByteCassette.cpp" "TXStream.cpp"
                       INCLUDE_DIRS "include"
                       REQUIRES MQTT_SubHandler esp-libopus)