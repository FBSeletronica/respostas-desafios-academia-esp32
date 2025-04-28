#include "led_rgb.h"
#include "driver/gpio.h"

#define LED_RED_GPIO   14
#define LED_GREEN_GPIO 13
#define LED_BLUE_GPIO  12

void led_rgb_init(void)
{
    gpio_set_direction(LED_RED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_GREEN_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(LED_BLUE_GPIO, GPIO_MODE_OUTPUT);

    gpio_set_level(LED_RED_GPIO, 0);
    gpio_set_level(LED_GREEN_GPIO, 0);
    gpio_set_level(LED_BLUE_GPIO, 0);
}

void led_rgb_set_color(uint8_t r, uint8_t g, uint8_t b)
{
    gpio_set_level(LED_RED_GPIO, r > 0 ? 1 : 0);
    gpio_set_level(LED_GREEN_GPIO, g > 0 ? 1 : 0);
    gpio_set_level(LED_BLUE_GPIO, b > 0 ? 1 : 0);
}
