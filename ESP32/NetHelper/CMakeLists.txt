

idf_component_register( SRCS "net_helpers.cpp"
                        INCLUDE_DIRS "include"
                        EMBED_TXTFILES lets-encrypt-r3-r4.pem
                        REQUIRES app_update esp_https_ota MQTT_SubHandler)