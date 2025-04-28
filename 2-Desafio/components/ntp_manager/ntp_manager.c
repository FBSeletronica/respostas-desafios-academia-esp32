#include "ntp_manager.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "time.h"

static const char *TAG = "NTP_MANAGER";

static bool time_synced = false;

static void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Horário sincronizado via NTP");
    time_synced = true;
}

void ntp_manager_start(void)
{
    ESP_LOGI(TAG, "Inicializando sincronização NTP...");
    setenv("TZ", "America/Sao_Paulo", 1);
    tzset();
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org"); // Pode mudar o servidor se quiser
    esp_sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    esp_sntp_init();
}

bool ntp_manager_is_time_synced(void)
{
    return time_synced;
}

bool ntp_manager_get_time(struct tm *time_info)
{
    if (!time_synced) {
        ESP_LOGW(TAG, "Tentativa de pegar o horário antes da sincronização");
        return false;
    }

    time_t now;
    time(&now);
    localtime_r(&now, time_info);
    return true;
}
