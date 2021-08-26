
if(CONFIG_BT_BLUEDROID_ENABLED)
	set(srcs "BLEHandler.cpp" "BLESlaveChannel.cpp" "Characteristic.cpp" "Service.cpp")
endif()

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "include"
                       REQUIRES bt)
