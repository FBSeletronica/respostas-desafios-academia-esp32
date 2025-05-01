idf_component_register(SRCS "alarm_manager.c"
                       INCLUDE_DIRS "include"
                       REQUIRES nvs_storage buzzer_manager led_rgb ntp_manager freertos)
