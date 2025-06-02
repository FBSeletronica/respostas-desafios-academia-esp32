idf_component_register(SRCS "node_relay.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_driver_gpio esp_timer esp_wifi nvs_flash common)