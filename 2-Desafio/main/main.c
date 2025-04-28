#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi_manager.h"
#include "ntp_manager.h"
#include "nvs_storage.h"
#include "alarm_manager.h"
#include "web_server.h"
#include "display_manager.h"
#include "buttons.h"
#include "buzzer.h"
#include "esp_spiffs.h"
#include "esp_log.h"

static const char *TAG = "MAIN";

void init_spiffs(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Falha ao montar ou formatar o SPIFFS");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Partição SPIFFS não encontrada");
        } else {
            ESP_LOGE(TAG, "Falha ao inicializar SPIFFS (%s)", esp_err_to_name(ret));
        }
    } else {
        size_t total = 0, used = 0;
        esp_spiffs_info(NULL, &total, &used);
        ESP_LOGI(TAG, "SPIFFS montado: Total: %d, Usado: %d", total, used);
    }
}

static void on_wifi_connected(void)
{
    ESP_LOGI(TAG, "Wi-Fi conectado! Sincronizando NTP...");
    ntp_manager_start();    // Sincroniza NTP
    init_spiffs();          // Monta SPIFFS
    web_server_start();     // Inicia o Web Server
}

void app_main(void)
{
    ESP_LOGI(TAG, "Inicializando armazenamento NVS...");
    nvs_storage_init();

    ESP_LOGI(TAG, "Inicializando Buzzer...");
    buzzer_init();

    ESP_LOGI(TAG, "Inicializando Wi-Fi Manager...");
    wifi_manager_init(NULL, NULL, on_wifi_connected);

    ESP_LOGI(TAG, "Inicializando Alarm Manager...");
    alarm_manager_init();

    ESP_LOGI(TAG, "Inicializando Display Manager...");
    display_manager_init();

    ESP_LOGI(TAG, "Inicializando Botões...");
    buttons_init();

    while (1) {
        if (wifi_manager_is_connected()) {
            if (ntp_manager_is_time_synced()) {
                struct tm timeinfo;
                if (ntp_manager_get_time(&timeinfo)) {
                    ESP_LOGI(TAG, "Horário atual: %02d:%02d:%02d - Dia: %02d",
                             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_wday);
                }
            } else {
                ESP_LOGI(TAG, "Aguardando sincronização do horário via NTP...");
            }
        } else {
            ESP_LOGW(TAG, "Wi-Fi não conectado!");
        }

        display_manager_update();
        display_manager_handle_buttons();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
