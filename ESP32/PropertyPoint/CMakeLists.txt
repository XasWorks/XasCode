
idf_component_register(SRCS "UARTOutput.cpp" "BLEOutput.cpp" "CustomProperty.cpp" "JSONProperty.cpp" "SingleProperty.cpp" "BaseHandler.cpp" "BaseOutput.cpp"
	"BaseProperty.cpp" "MQTTOutput.cpp"
	"SingleProperty.cpp" "JSONProperty.cpp"
	"CustomProperty.cpp"
	INCLUDE_DIRS "include"
	REQUIRES json MQTT_SubHandler NeoController xnm-ble)