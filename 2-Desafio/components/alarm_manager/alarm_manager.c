#include "alarm_manager.h"
#include "nvs_storage.h"
#include "buzzer.h"
#include "led_rgb.h"
#include "ntp_manager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"
#include <string.h>

#define CHECK_INTERVAL_MS (1000) // Verifica alarmes a cada 1 segundo

static const char *TAG = "ALARM_MANAGER";

// Definições internas
static alarm_t alarm_list[MAX_ALARMS];
static int num_alarms = 0;
static int last_triggered_minute = -1;
static bool alarms_enabled = true;

static void check_alarms(void)
{
    if (!alarms_enabled) {
        return;
    }

    if (!ntp_manager_is_time_synced()) {
        ESP_LOGW(TAG, "Hora ainda não sincronizada. Ignorando checagem de alarmes.");
        return;
    }

    struct tm now;
    if (!ntp_manager_get_time(&now)) {
        ESP_LOGW(TAG, "Erro ao obter horário atual.");
        return;
    }

    if (now.tm_min == last_triggered_minute) {
        
        return;
    }

    ESP_LOGI(TAG, "Verificando alarmes para %02d:%02d, Dia %d", now.tm_hour, now.tm_min, now.tm_wday);

    for (int i = 0; i < num_alarms; i++) {
        if (alarm_list[i].hour == now.tm_hour &&
            alarm_list[i].minute == now.tm_min &&
            alarm_list[i].weekday == now.tm_wday) {

            ESP_LOGI(TAG, "Alarme encontrado! Executando...");

            buzzer_play_melody((buzzer_melody_t)alarm_list[i].melody);
            led_rgb_set_color(0, 255, 0); 
            vTaskDelay(pdMS_TO_TICKS(1000));
            led_rgb_set_color(0, 0, 0);

            last_triggered_minute = now.tm_min;
            return; 
        }
    }
}

static void alarm_manager_task(void *param)
{
    while (1) {
        check_alarms();
        vTaskDelay(pdMS_TO_TICKS(CHECK_INTERVAL_MS));
    }
}

void alarm_manager_init(void)
{
    ESP_LOGI(TAG, "Inicializando Alarm Manager...");

    num_alarms = nvs_storage_load_alarms(alarm_list, MAX_ALARMS);
    ESP_LOGI(TAG, "Carregados %d alarmes.", num_alarms);

    xTaskCreate(alarm_manager_task, "alarm_manager_task", 4096, NULL, 5, NULL);
}


bool alarm_manager_is_enabled(void)
{
    return alarms_enabled;
}

int alarm_manager_count(void)
{
    return num_alarms;
}

bool alarm_manager_get_alarm(int index, alarm_t *alarm)
{
    if (index < 0 || index >= num_alarms) {
        return false;
    }
    *alarm = alarm_list[index];
    return true;
}
