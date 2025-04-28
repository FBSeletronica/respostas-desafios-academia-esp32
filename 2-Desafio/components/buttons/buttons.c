#include "buttons.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>

#define DEBOUNCE_DELAY_MS 30

// Armazena o último tempo de pressionamento para cada botão
static uint32_t last_press_tick[GPIO_NUM_MAX] = {0};

void buttons_init(void)
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask =
            (1ULL << BUTTON_1) |
            (1ULL << BUTTON_2) |
            (1ULL << BUTTON_3) |
            (1ULL << BUTTON_4) |
            (1ULL << BUTTON_5) |
            (1ULL << BUTTON_6),
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    gpio_config(&io_conf);
}

static bool button_debounced(gpio_num_t button)
{
    bool pressed = (gpio_get_level(button) == 0);

    if (pressed) {
        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
        if ((now - last_press_tick[button]) > DEBOUNCE_DELAY_MS) {
            last_press_tick[button] = now;
            return true;
        }
    }
    return false;
}

// Funções públicas para cada botão:

bool button_up_pressed(void)
{
    return button_debounced(BUTTON_1);
}

bool button_left_pressed(void)
{
    return button_debounced(BUTTON_2);
}

bool button_right_pressed(void)
{
    return button_debounced(BUTTON_3);
}

bool button_down_pressed(void)
{
    return button_debounced(BUTTON_4);
}

bool button_enter_pressed(void)
{
    return button_debounced(BUTTON_5);
}

bool button_emergency_pressed(void)
{
    return button_debounced(BUTTON_6);
}


void wait_for_button_release(gpio_num_t button)
{
    TickType_t start_tick = xTaskGetTickCount();
    const TickType_t timeout_ticks = pdMS_TO_TICKS(2000); // 2 segundos de timeout máximo
    const TickType_t check_interval = pdMS_TO_TICKS(10);  // Verificar a cada 10ms

    while (gpio_get_level(button) == 0)
    {
        vTaskDelay(check_interval);

        // Se passou o tempo máximo de espera, sai da função
        if ((xTaskGetTickCount() - start_tick) > timeout_ticks) {
            break;
        }
    }
}