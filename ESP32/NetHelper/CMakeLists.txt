

idf_component_register( SRCS "wifi.cpp" "ota.cpp" "net_helpers.cpp"
                        INCLUDE_DIRS "include"
                        EMBED_TXTFILES lets-encrypt-r3-r4.pem
                        REQUIRES app_update json esp_https_ota esp_wifi MQTT_SubHandler xnm-ble)