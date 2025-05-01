#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "button_manager.h"
#include "buzzer_manager.h"
#include "display_manager.h"
#include "alarm_manager.h"
#include "ntp_manager.h"
#include "wifi_manager.h"
#include "nvs_storage.h"
#include "web_server.h"
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
    // Inicialização dos módulos

    ESP_LOGI(TAG, "Inicializando armazenamento NVS...");
    nvs_storage_init();

    ESP_LOGI(TAG, "Inicializando Wi-Fi Manager...");
    wifi_manager_init(NULL, NULL, on_wifi_connected);

    ESP_LOGI(TAG, "Inicializando Botões...");
    button_manager_init();

    ESP_LOGI(TAG, "Inicializando Buzzer...");
    buzzer_manager_init();
    //ntp_manager_start();

    ESP_LOGI(TAG, "Inicializando Alarm Manager...");
    alarm_manager_init();

    ESP_LOGI(TAG, "Inicializando Display Manager...");
    display_manager_init();

    // Registra botões da Franzininho WiFi LAB01
    button_manager_add_button(GPIO_NUM_7, BUTTON_PULL_UP, NULL); // BT_UP
    button_manager_add_button(GPIO_NUM_6, BUTTON_PULL_UP, NULL); // BT_LEFT
    button_manager_add_button(GPIO_NUM_5, BUTTON_PULL_UP, NULL); // BT_RIGHT
    button_manager_add_button(GPIO_NUM_4, BUTTON_PULL_UP, NULL); // BT_DOWN
    button_manager_add_button(GPIO_NUM_3, BUTTON_PULL_UP, NULL); // BT_B
    button_manager_add_button(GPIO_NUM_2, BUTTON_PULL_UP, NULL); // BT_A

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

        // Emergência: BT_B (GPIO_NUM_3)
        if (button_manager_is_pressed(GPIO_NUM_3)) {
            display_manager_set_screen(SCREEN_EMERGENCY);
            //buzzer_manager_play(BUZZER_MELODY_ALARM, 10000);
            buzzer_manager_play(BUZZER_MELODY_NORMAL, 10000);
            vTaskDelay(pdMS_TO_TICKS(100)); // debounce
            continue;
        }

        screen_t screen = display_manager_get_screen();

        switch (screen) {
            case SCREEN_MAIN:
                if (button_manager_is_pressed(GPIO_NUM_2)) {  // BT_A = ENTER
                    display_manager_set_screen(SCREEN_MENU);
                    buzzer_manager_play(BUZZER_MELODY_BEEP, 100);
                }
                break;

            case SCREEN_MENU:
                if (button_manager_is_pressed(GPIO_NUM_4)) { // BT_DOWN
                    display_manager_next_menu();
                    buzzer_manager_play(BUZZER_MELODY_BEEP, 100);
                }
                if (button_manager_is_pressed(GPIO_NUM_7)) { // BT_UP
                    display_manager_prev_menu();
                    buzzer_manager_play(BUZZER_MELODY_BEEP, 100);
                }
                if (button_manager_is_pressed(GPIO_NUM_2)) { // BT_A = ENTER
                    if (display_manager_get_menu_index() == 0) {
                        display_manager_set_screen(SCREEN_ALARMS);
                    } else {
                        display_manager_set_screen(SCREEN_MAIN);
                    }
                    buzzer_manager_play(BUZZER_MELODY_BEEP, 100);
                }
                if (button_manager_is_pressed(GPIO_NUM_6)) { // BT_LEFT = VOLTAR
                    display_manager_set_screen(SCREEN_MAIN);
                    buzzer_manager_play(BUZZER_MELODY_BEEP, 100);
                }
                break;

            case SCREEN_ALARMS:
                if (button_manager_is_pressed(GPIO_NUM_4)) { // BT_DOWN
                    display_manager_next_alarm();
                    buzzer_manager_play(BUZZER_MELODY_BEEP, 100);
                }
                if (button_manager_is_pressed(GPIO_NUM_7)) { // BT_UP
                    display_manager_prev_alarm();
                    buzzer_manager_play(BUZZER_MELODY_BEEP, 100);
                }
                if (button_manager_is_pressed(GPIO_NUM_2) || button_manager_is_pressed(GPIO_NUM_6)) {
                    display_manager_set_screen(SCREEN_MENU);
                    buzzer_manager_play(BUZZER_MELODY_BEEP, 100);
                }
                break;

            case SCREEN_EMERGENCY:
                if (button_manager_is_pressed(GPIO_NUM_2) || button_manager_is_pressed(GPIO_NUM_6)) {
                    display_manager_set_screen(SCREEN_MAIN);
                    buzzer_manager_stop();
                    buzzer_manager_play(BUZZER_MELODY_BEEP, 100);
                }
                break;

            default:
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Loop interval
    }
}


