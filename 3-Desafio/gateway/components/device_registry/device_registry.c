#include "device_registry.h"
#include "common.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include <string.h>

#define NVS_NAMESPACE "dev_registry"
#define NVS_KEY "devices"

static const char *TAG = "device_registry";

static device_info_t registry[MAX_DEVICES];
static size_t registry_size = 0;

static nvs_handle_t s_nvs_handle;

// Carrega os dispositivos salvos da NVS
static void load_registry_from_nvs()
{
    size_t required_size = sizeof(registry);
    esp_err_t err = nvs_get_blob(s_nvs_handle, NVS_KEY, registry, &required_size);
    if (err == ESP_OK) {
        registry_size = required_size / sizeof(device_info_t);
        ESP_LOGI(TAG, "Carregados %d dispositivos do registro", registry_size);
    } else {
        ESP_LOGI(TAG, "Nenhum registro salvo na NVS");
        registry_size = 0;
    }
}

// Inicializa o registro de dispositivos
esp_err_t device_registry_init(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(nvs_open(NVS_NAMESPACE, NVS_READWRITE, &s_nvs_handle));
    load_registry_from_nvs();
    return ESP_OK;
}

// Procura um dispositivo pelo endereço MAC
status_t device_registry_find(const uint8_t *mac_addr, device_info_t *device_out)
{
    for (size_t i = 0; i < registry_size; ++i) {
        if (memcmp(registry[i].mac, mac_addr, 6) == 0) {
            if (device_out) *device_out = registry[i];
            return STATUS_OK;
        }
    }
    return STATUS_NOT_FOUND;
}

// Procura um dispositivo pelo tipo e ID
status_t device_registry_find_by_type_id(device_type_t type, uint8_t id, device_info_t *device_out)
{
    for (size_t i = 0; i < registry_size; ++i) {
        if (registry[i].type == type && registry[i].id == id) {
            if (device_out) *device_out = registry[i];
            return STATUS_OK;
        }
    }
    return STATUS_NOT_FOUND;
}

// Adiciona um novo dispositivo ao registro
status_t device_registry_add(const device_info_t *device)
{
    if (registry_size >= MAX_DEVICES) {
        ESP_LOGW(TAG, "Registro cheio");
        return STATUS_ERROR;
    }

    // Verifica se já existe
    if (device_registry_find(device->mac, NULL) == STATUS_OK) {
        ESP_LOGW(TAG, "Dispositivo já registrado");
        return STATUS_ERROR;
    }

    registry[registry_size++] = *device;
    device_registry_save_all();
    ESP_LOGI(TAG, "Novo dispositivo registrado: tipo=%d id=%d", device->type, device->id);
    return STATUS_OK;
}

// Salva todos os dispositivos no registro para a NVS
esp_err_t device_registry_save_all(void)
{
    esp_err_t err = nvs_set_blob(s_nvs_handle, NVS_KEY, registry, registry_size * sizeof(device_info_t));
    if (err == ESP_OK) {
        err = nvs_commit(s_nvs_handle);
    }
    return err;
}
