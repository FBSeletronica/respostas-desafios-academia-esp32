idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES buzzer led_rgb buttons wifi_manager ntp_manager nvs_storage alarm_manager web_server spiffs display_manager
                    )


spiffs_create_partition_image(spiffs ../spiffs FLASH_IN_PROJECT)