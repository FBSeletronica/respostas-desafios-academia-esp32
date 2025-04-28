idf_component_register(SRCS "display_manager.c"
                       INCLUDE_DIRS "include"
                       REQUIRES ssd1306 buttons ntp_manager alarm_manager)