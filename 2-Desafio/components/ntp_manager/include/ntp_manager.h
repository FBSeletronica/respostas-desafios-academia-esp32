#pragma once

#include <stdbool.h>
#include <time.h>  // <-- ADICIONAR ESTA LINHA AQUI

typedef void (*wifi_connected_callback_t)(void);

void ntp_manager_start(void);
bool ntp_manager_is_time_synced(void);
bool ntp_manager_get_time(struct tm *time_info);
