#pragma once

typedef enum {
    BUZZER_MELODY_NORMAL = 0,
    BUZZER_MELODY_INTERVAL,
    BUZZER_MELODY_EMERGENCY,
    BUZZER_MELODY_SPECIAL
} buzzer_melody_t;

void buzzer_init(void);
void buzzer_play_melody(buzzer_melody_t melody_id); 
void buzzer_start_melody(buzzer_melody_t melody_id, uint32_t duration_ms);
void buzzer_stop(void);
