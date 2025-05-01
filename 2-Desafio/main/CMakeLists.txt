idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES 
        button_manager
        buzzer_manager
        display_manager
        alarm_manager
        ntp_manager
        wifi_manager
        led_rgb
        nvs_storage
        web_server
        spiffs
)

spiffs_create_partition_image(spiffs ../spiffs FLASH_IN_PROJECT)