
list(APPEND srcs "MasterAction.cpp"
				"MAX11613.cpp"
				"AS1115.cpp"
				"BME680.cpp"
				"LSM6DS3.cpp"
				"LT303ALS.cpp"
				"DRV2605.cpp")

idf_component_register(SRCS 	"${srcs}"
                       INCLUDE_DIRS "include")
