#pragma once

#include "driver/gpio.h"
#include "esp_err.h"
#include "button_hal.h"

typedef void (*button_callback_t)(gpio_num_t gpio);

esp_err_t button_manager_add_button(gpio_num_t gpio, button_pull_t pull, button_callback_t callback);
void button_manager_init(void);
bool button_manager_is_pressed(gpio_num_t gpio);
