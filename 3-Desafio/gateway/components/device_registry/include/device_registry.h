#ifndef DEVICE_REGISTRY_H
#define DEVICE_REGISTRY_H

#include "common.h"
#include "esp_err.h"

// Inicializa o registro e carrega da NVS
esp_err_t device_registry_init(void);

// Busca por MAC address
status_t device_registry_find(const uint8_t *mac_addr, device_info_t *device_out);

// Busca por tipo e ID
status_t device_registry_find_by_type_id(device_type_t type, uint8_t id, device_info_t *device_out);

// Adiciona novo dispositivo (se ainda não existe)
status_t device_registry_add(const device_info_t *device);

// Salva todos os dispositivos registrados na NVS
esp_err_t device_registry_save_all(void);

#endif // DEVICE_REGISTRY_H
