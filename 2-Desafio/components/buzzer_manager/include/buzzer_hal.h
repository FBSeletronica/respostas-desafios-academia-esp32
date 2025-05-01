#pragma once

void buzzer_hal_init(void);
void buzzer_hal_play_note(int freq_hz, int duration_ms);
void buzzer_hal_stop(void);
