idf_component_register(SRCS "display_manager.c"
                       INCLUDE_DIRS "include"
                       REQUIRES ssd1306 button_manager ntp_manager alarm_manager)