#pragma once

#include "esp_err.h"
#include <stdbool.h>

typedef enum {
    BUZZER_MELODY_NONE = 0,
    BUZZER_MELODY_BEEP,
    BUZZER_MELODY_NORMAL,
    BUZZER_MELODY_ALARM,
    BUZZER_MELODY_SPECIAL
} buzzer_melody_t;

void buzzer_manager_init(void);
esp_err_t buzzer_manager_play(buzzer_melody_t melody, uint32_t duration_ms);
void buzzer_manager_stop(void);
bool buzzer_manager_is_playing(void);
