
if(CONFIG_BT_BLUEDROID_ENABLED)
	set(srcs "SPPData.cpp" "SPPServer.cpp" "SPPStream.cpp" "SPPValue.cpp")
endif()

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "include"
                       REQUIRES bt)
