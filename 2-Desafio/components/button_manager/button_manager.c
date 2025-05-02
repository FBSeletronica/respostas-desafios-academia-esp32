#include "button_manager.h"
#include "button_hal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

#define MAX_BUTTONS 10
#define DEBOUNCE_TIME_MS 50

typedef struct {
    gpio_num_t gpio_num;
    button_pull_t pull;
    button_callback_t callback;
    bool last_state;
    uint32_t last_change_time;
    bool pressed_flag;
} button_t;

static button_t buttons[MAX_BUTTONS];
static int button_count = 0;

static bool is_debounced_change(button_t *btn, bool current_state, uint32_t now_ms)
{
    if (current_state != btn->last_state) {
        if (now_ms - btn->last_change_time > DEBOUNCE_TIME_MS) {
            btn->last_change_time = now_ms;
            btn->last_state = current_state;
            return true;
        }
    }
    return false;
}

static void button_manager_task(void *arg)
{
    while (1) {
        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
        for (int i = 0; i < button_count; i++) {
            bool pressed = button_hal_is_pressed(buttons[i].gpio_num);
            if (is_debounced_change(&buttons[i], pressed, now)) {
                if (pressed) {
                    buttons[i].pressed_flag = true;
                    if (buttons[i].callback) {
                        buttons[i].callback(buttons[i].gpio_num);
                    }
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

esp_err_t button_manager_add_button(gpio_num_t gpio, button_pull_t pull, button_callback_t callback)
{
    if (button_count >= MAX_BUTTONS) return ESP_ERR_NO_MEM;

    button_hal_configure(gpio, pull);

    buttons[button_count].gpio_num = gpio;
    buttons[button_count].pull = pull;
    buttons[button_count].callback = callback;
    buttons[button_count].last_state = false;
    buttons[button_count].last_change_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
    buttons[button_count].pressed_flag = false;

    button_count++;
    return ESP_OK;
}

void button_manager_init(void)
{
    xTaskCreate(button_manager_task, "button_manager_task", 2048, NULL, 5, NULL);
}

bool button_manager_is_pressed(gpio_num_t gpio)
{
    return button_hal_is_pressed(gpio);
}

bool button_manager_was_pressed(gpio_num_t gpio)
{
    for (int i = 0; i < button_count; i++) {
        if (buttons[i].gpio_num == gpio) {
            if (buttons[i].pressed_flag) {
                buttons[i].pressed_flag = false;
                return true;
            }
        }
    }
    return false;
}