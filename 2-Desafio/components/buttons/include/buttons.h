#pragma once

#include "driver/gpio.h"
#include <stdbool.h>

// Mapas corretos dos botões para GPIO
#define BUTTON_6 GPIO_NUM_2  // BT6 - Emergência
#define BUTTON_5 GPIO_NUM_3  // BT5 - ENTER
#define BUTTON_4 GPIO_NUM_4  // BT4 - DOWN
#define BUTTON_3 GPIO_NUM_5  // BT3 - RIGHT
#define BUTTON_2 GPIO_NUM_6  // BT2 - LEFT
#define BUTTON_1 GPIO_NUM_7  // BT1 - UP

void buttons_init(void);

bool button_up_pressed(void);
bool button_down_pressed(void);
bool button_left_pressed(void);
bool button_right_pressed(void);
bool button_enter_pressed(void);
bool button_emergency_pressed(void);
void wait_for_button_release(gpio_num_t button);
