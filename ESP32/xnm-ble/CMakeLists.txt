

if(CONFIG_BT_NIMBLE_ENABLED)
	set(srcs "host.cpp" "service.cpp" "string_stream.cpp" "battery_svc.cpp")
endif()

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "include"
                       REQUIRES bt)
