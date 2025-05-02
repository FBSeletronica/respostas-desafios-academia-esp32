#pragma once

#include "esp_err.h"
#include <stdbool.h>

typedef enum {
    ALARM_AUDIO_TYPE_NORMAL,
    ALARM_AUDIO_TYPE_INTERVAL,
    ALARM_AUDIO_TYPE_EMERGENCY,
    ALARM_AUDIO_TYPE_SPECIAL
} alarm_audio_type_t;

void alarm_audio_manager_init(void);
esp_err_t alarm_audio_manager_play(alarm_audio_type_t type);
void alarm_audio_manager_stop(void);
bool alarm_audio_manager_is_playing(void);
