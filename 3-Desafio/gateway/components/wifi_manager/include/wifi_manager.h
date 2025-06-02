#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <stdbool.h>
#include "esp_err.h"

// Funções públicas
esp_err_t wifi_manager_init(void);
bool wifi_manager_is_connected(void);

#endif // WIFI_MANAGER_H
