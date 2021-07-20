
idf_component_register(SRCS "SingleProperty.cpp" "BaseHandler.cpp" "BaseOutput.cpp"
	"BaseProperty.cpp" "MQTTOutput.cpp"
	"SingleProperty.cpp"
	INCLUDE_DIRS "include"
	REQUIRES json MQTT_SubHandler NeoController)