idf_component_register(SRCS "Handler.cpp" "Subscription.cpp"
                       INCLUDE_DIRS "include"
                       REQUIRES mqtt esp_wifi wpa_supplicant nvs_flash)
