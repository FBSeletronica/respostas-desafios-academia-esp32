idf_component_register(SRCS "gateway.c"
                    INCLUDE_DIRS ""
                    REQUIRES json
                    REQUIRES common device_registry espnow_manager mqtt_manager wifi_manager
                    
                    )


