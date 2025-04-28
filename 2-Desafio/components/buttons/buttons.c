#include "buttons.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define DEBOUNCE_TIME_MS 50

typedef struct {
    gpio_num_t gpio;
    bool last_state;
    uint32_t last_change_time;
} button_t;

static button_t buttons[] = {
    { BUTTON_1, true, 0 },
    { BUTTON_2, true, 0 },
    { BUTTON_3, true, 0 },
    { BUTTON_4, true, 0 },
    { BUTTON_5, true, 0 },
    { BUTTON_6, true, 0 }
};

void buttons_init(void)
{
    for (int i = 0; i < sizeof(buttons)/sizeof(buttons[0]); i++) {
        gpio_set_direction(buttons[i].gpio, GPIO_MODE_INPUT);
        gpio_set_pull_mode(buttons[i].gpio, GPIO_PULLUP_ONLY);
    }
}

static bool is_button_pressed(button_t *button)
{
    bool current_state = gpio_get_level(button->gpio);
    uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;

    if (current_state != button->last_state) {
        if ((now - button->last_change_time) > DEBOUNCE_TIME_MS) {
            button->last_state = current_state;
            button->last_change_time = now;
            if (current_state == 0) {
                return true;                // Botão pressionado (ativo em LOW)
            }
        }
    }
    return false;
}

bool button_up_pressed(void)       { return is_button_pressed(&buttons[0]); }
bool button_left_pressed(void)     { return is_button_pressed(&buttons[1]); }
bool button_right_pressed(void)    { return is_button_pressed(&buttons[2]); }
bool button_down_pressed(void)     { return is_button_pressed(&buttons[3]); }
bool button_enter_pressed(void)    { return is_button_pressed(&buttons[4]); }
bool button_emergency_pressed(void){ return is_button_pressed(&buttons[5]); }
