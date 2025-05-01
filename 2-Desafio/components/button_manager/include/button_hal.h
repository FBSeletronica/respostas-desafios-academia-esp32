#pragma once

#include "driver/gpio.h"
#include <stdbool.h>

typedef enum {
    BUTTON_PULL_NONE,
    BUTTON_PULL_UP,
    BUTTON_PULL_DOWN
} button_pull_t;

void button_hal_configure(gpio_num_t gpio, button_pull_t pull);
bool button_hal_is_pressed(gpio_num_t gpio);
