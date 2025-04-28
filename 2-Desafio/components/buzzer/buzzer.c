#include <stdint.h>
#include "buzzer.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define BUZZER_GPIO 17

typedef struct {
    buzzer_melody_t melody_id;
    uint32_t duration_ms;
} buzzer_task_param_t;

typedef struct {
    int freq_hz;
    int duration_ms;
} note_t;

// Melodias
static const note_t melody_normal[] = {
    { 1000, 200 }, { 0, 100 }, { 1000, 200 }, { 0, 100 }, { 1000, 200 }, { -1, 0 }
};

static const note_t melody_interval[] = {
    { 800, 500 }, { 0, 200 }, { 800, 500 }, { -1, 0 }
};

static const note_t melody_emergency[] = {
    { 1500, 100 }, { 0, 50 }, { 1500, 100 }, { 0, 50 }, { 1500, 100 }, { 0, 50 }, { 1500, 100 }, { -1, 0 }
};

static const note_t melody_special[] = {
    { 600, 200 }, { 800, 200 }, { 1000, 200 }, { 1200, 400 }, { -1, 0 }
};

// Controle de execução
static TaskHandle_t buzzer_task_handle = NULL;
static volatile bool buzzer_stop_requested = false;

void buzzer_init(void)
{
    gpio_set_direction(BUZZER_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(BUZZER_GPIO, 0);

    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 2000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_conf);

    ledc_channel_config_t channel_conf = {
        .gpio_num = BUZZER_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&channel_conf);
}

static void buzzer_play_note(int freq_hz, int duration_ms)
{
    if (freq_hz == 0) {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    } else {
        ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, freq_hz);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 512);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    }
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
}

// 🔥 Task do Buzzer (agora corrigida)
static void buzzer_task(void *pvParameter)
{
    buzzer_task_param_t *param = (buzzer_task_param_t *) pvParameter;
    buzzer_melody_t melody_id = param->melody_id;
    uint32_t duration_ms = param->duration_ms;

    const note_t *melody = NULL;
    switch (melody_id) {
        case BUZZER_MELODY_NORMAL: melody = melody_normal; break;
        case BUZZER_MELODY_INTERVAL: melody = melody_interval; break;
        case BUZZER_MELODY_EMERGENCY: melody = melody_emergency; break;
        case BUZZER_MELODY_SPECIAL: melody = melody_special; break;
        default: melody = melody_normal; break;
    }

    uint32_t elapsed_ms = 0;

    while (!buzzer_stop_requested && elapsed_ms < duration_ms) {
        for (int i = 0; melody[i].freq_hz != -1 && elapsed_ms < duration_ms; i++) {
            if (buzzer_stop_requested) {
                break;
            }
            buzzer_play_note(melody[i].freq_hz, melody[i].duration_ms);
            elapsed_ms += melody[i].duration_ms;
        }
    }

    // Garante parar o som no final
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

    buzzer_task_handle = NULL;
    free(param); // Libera memória alocada
    vTaskDelete(NULL);
}

// API para tocar melodia tradicional (sem tempo definido)
void buzzer_play_melody(buzzer_melody_t melody_id)
{
    buzzer_start_melody(melody_id, 5000); // 5 segundos padrão, ou escolha outro tempo
}

// API para tocar melodia com duração controlada
void buzzer_start_melody(buzzer_melody_t melody_id, uint32_t duration_ms)
{
    if (buzzer_task_handle != NULL) {
        return; // Já tem buzzer tocando
    }

    buzzer_stop_requested = false;

    buzzer_task_param_t *param = malloc(sizeof(buzzer_task_param_t));
    if (param == NULL) {
        return;
    }

    param->melody_id = melody_id;
    param->duration_ms = duration_ms;

    xTaskCreatePinnedToCore(buzzer_task, "buzzer_task", 2048, param, 5, &buzzer_task_handle, tskNO_AFFINITY);
}

// API para parar o buzzer
void buzzer_stop(void)
{
    buzzer_stop_requested = true;
}
