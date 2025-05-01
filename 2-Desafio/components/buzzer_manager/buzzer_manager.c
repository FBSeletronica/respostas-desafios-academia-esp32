#include "buzzer_manager.h"
#include "buzzer_hal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdlib.h>

#define TAG "BUZZER_MANAGER"

typedef struct {
    int freq;
    int duration_ms;
} buzzer_note_t;

static const buzzer_note_t melody_beep[] = {
    { 1000, 100 }, { -1, 0 }
};

static const buzzer_note_t melody_normal[] = {
    { 880, 150 }, { 0, 100 },
    { 880, 150 }, { 0, 100 },
    { 988, 200 }, { 0, 300 },
    { -1, 0 },
};

static const buzzer_note_t melody_alarm[] = {
    { 1200, 200 }, { 0, 100 }, { 1200, 200 }, { -1, 0 }
};

static const buzzer_note_t melody_special[] = {
    { 800, 100 }, { 1000, 100 }, { 1200, 100 }, { -1, 0 }
};

static TaskHandle_t buzzer_task_handle = NULL;
static volatile bool stop_requested = false;

static const buzzer_note_t* get_melody(buzzer_melody_t melody) {
    switch (melody) {
        case BUZZER_MELODY_BEEP: return melody_beep;
        case BUZZER_MELODY_NORMAL: return melody_normal;
        case BUZZER_MELODY_ALARM: return melody_alarm;
        case BUZZER_MELODY_SPECIAL: return melody_special;
        default: return NULL;
    }
}

static void buzzer_task(void *param) {
    buzzer_melody_t melody = (buzzer_melody_t)(intptr_t)param;
    const buzzer_note_t *notes = get_melody(melody);

    uint32_t elapsed = 0;
    const uint32_t max_duration = 10000; // fallback

    while (notes && notes->freq != -1 && !stop_requested && elapsed < max_duration) {
        buzzer_hal_play_note(notes->freq, notes->duration_ms);
        vTaskDelay(pdMS_TO_TICKS(notes->duration_ms));
        elapsed += notes->duration_ms;
        notes++;
    }

    buzzer_hal_stop();
    buzzer_task_handle = NULL;
    vTaskDelete(NULL);
}

void buzzer_manager_init(void) {
    buzzer_hal_init();
}

esp_err_t buzzer_manager_play(buzzer_melody_t melody, uint32_t duration_ms) {
    if (buzzer_task_handle != NULL) return ESP_FAIL;

    stop_requested = false;
    xTaskCreate(buzzer_task, "buzzer_task", 2048, (void *)(intptr_t)melody, 5, &buzzer_task_handle);
    return ESP_OK;
}

void buzzer_manager_stop(void) {
    stop_requested = true;
}

bool buzzer_manager_is_playing(void) {
    return buzzer_task_handle != NULL;
}
