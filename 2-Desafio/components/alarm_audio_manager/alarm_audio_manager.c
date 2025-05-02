#include "alarm_audio_manager.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "driver/dac_continuous.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <stdio.h>

#define TAG "ALARM_AUDIO_MANAGER"
#define AUDIO_SAMPLE_RATE_HZ 11025
#define AUDIO_BUFFER_SIZE 1024
#define WAV_HEADER_SIZE 44

static dac_continuous_handle_t dac_handle = NULL;
static bool is_playing = false;
static TaskHandle_t audio_task_handle = NULL;

static const char *audio_paths[] = {
    [ALARM_AUDIO_TYPE_NORMAL] = "/spiffs/audio/normal.wav",
    [ALARM_AUDIO_TYPE_INTERVAL] = "/spiffs/audio/interval.wav",
    [ALARM_AUDIO_TYPE_EMERGENCY] = "/spiffs/audio/emergency.wav",
    [ALARM_AUDIO_TYPE_SPECIAL] = "/spiffs/audio/special.wav"
};

static void audio_task(void *param) {
    alarm_audio_type_t type = (alarm_audio_type_t)(intptr_t)param;

    const char *path = audio_paths[type];
    FILE *f = fopen(path, "rb");
    if (!f) {
        ESP_LOGE(TAG, "Erro ao abrir arquivo: %s", path);
        is_playing = false;
        audio_task_handle = NULL;
        vTaskDelete(NULL);
        return;
    }

    fseek(f, WAV_HEADER_SIZE, SEEK_SET);
    uint8_t buffer[AUDIO_BUFFER_SIZE];
    size_t bytes_read;
    is_playing = true;

    while ((bytes_read = fread(buffer, 1, AUDIO_BUFFER_SIZE, f)) > 0 && is_playing) {
        esp_err_t err = dac_continuous_write(dac_handle, buffer, bytes_read, NULL, -1);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Erro ao escrever no DAC: %s", esp_err_to_name(err));
            break;
        }
    }

    fclose(f);
    is_playing = false;
    audio_task_handle = NULL;
    vTaskDelete(NULL);
}

void alarm_audio_manager_init(void) {
    dac_continuous_config_t cfg = {
        .chan_mask = DAC_CHANNEL_MASK_CH1,
        .desc_num = 4,
        .buf_size = 2048,
        .freq_hz = AUDIO_SAMPLE_RATE_HZ,
        .offset = 0,
        .clk_src = DAC_DIGI_CLK_SRC_APLL,
        .chan_mode = DAC_CHANNEL_MODE_SIMUL,
    };

    esp_err_t err = dac_continuous_new_channels(&cfg, &dac_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Erro ao criar canais DAC: %s", esp_err_to_name(err));
        return;
    }

    ESP_ERROR_CHECK(dac_continuous_enable(dac_handle));
}

esp_err_t alarm_audio_manager_play(alarm_audio_type_t type) {
    if (is_playing || audio_task_handle != NULL) {
        ESP_LOGW(TAG, "Áudio já está sendo reproduzido");
        return ESP_FAIL;
    }

    if (type < 0 || type >= sizeof(audio_paths) / sizeof(audio_paths[0])) {
        ESP_LOGE(TAG, "Tipo de áudio inválido");
        return ESP_ERR_INVALID_ARG;
    }

    return xTaskCreate(audio_task, "audio_task", 4096, (void *)(intptr_t)type, 5, &audio_task_handle) == pdPASS
           ? ESP_OK : ESP_FAIL;
}

void alarm_audio_manager_stop(void) {
    is_playing = false;
}

bool alarm_audio_manager_is_playing(void) {
    return is_playing;
}
