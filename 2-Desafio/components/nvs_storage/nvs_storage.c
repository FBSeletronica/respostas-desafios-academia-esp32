#include "nvs_storage.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "NVS_STORAGE";
static const char *NAMESPACE = "alarms";
static const char *KEY = "alarm_list";

void nvs_storage_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

bool nvs_storage_save_alarm(const alarm_t *alarm)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erro ao abrir NVS: %s", esp_err_to_name(err));
        return false;
    }

    // Primeiro carrega a lista atual
    alarm_t alarms[MAX_ALARMS];
    size_t required_size = sizeof(alarms);
    err = nvs_get_blob(handle, KEY, alarms, &required_size);
    int num_alarms = (err == ESP_OK) ? required_size / sizeof(alarm_t) : 0;

    if (num_alarms >= MAX_ALARMS) {
        ESP_LOGW(TAG, "Limite máximo de alarmes atingido");
        nvs_close(handle);
        return false;
    }

    // Adiciona o novo alarme
    alarms[num_alarms] = *alarm;
    num_alarms++;

    // Salva de volta
    err = nvs_set_blob(handle, KEY, alarms, num_alarms * sizeof(alarm_t));
    if (err == ESP_OK) {
        nvs_commit(handle);
        nvs_close(handle);
        return true;
    } else {
        ESP_LOGE(TAG, "Erro ao salvar alarme: %s", esp_err_to_name(err));
        nvs_close(handle);
        return false;
    }
}

int nvs_storage_load_alarms(alarm_t *alarms, int max_alarms)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NAMESPACE, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Nenhum alarme salvo ainda.");
        return 0;
    }

    size_t required_size = max_alarms * sizeof(alarm_t);
    err = nvs_get_blob(handle, KEY, alarms, &required_size);
    nvs_close(handle);

    if (err == ESP_OK) {
        return required_size / sizeof(alarm_t);
    } else {
        ESP_LOGW(TAG, "Erro ao carregar alarmes: %s", esp_err_to_name(err));
        return 0;
    }
}

bool nvs_storage_clear_alarms(void)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erro ao abrir NVS para limpar: %s", esp_err_to_name(err));
        return false;
    }

    err = nvs_erase_key(handle, KEY);
    if (err == ESP_OK || err == ESP_ERR_NVS_NOT_FOUND) {
        nvs_commit(handle);
        nvs_close(handle);
        return true;
    } else {
        ESP_LOGE(TAG, "Erro ao limpar alarmes: %s", esp_err_to_name(err));
        nvs_close(handle);
        return false;
    }
}
