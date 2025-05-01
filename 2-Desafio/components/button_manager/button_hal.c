#include "button_hal.h"
#include "driver/gpio.h"

void button_hal_configure(gpio_num_t gpio, button_pull_t pull)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << gpio,
        .mode = GPIO_MODE_INPUT,
        .intr_type = GPIO_INTR_DISABLE
    };

    switch (pull) {
        case BUTTON_PULL_UP:
            io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
        case BUTTON_PULL_DOWN:
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
            break;
        default:
            io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
            io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
            break;
    }

    gpio_config(&io_conf);
}

bool button_hal_is_pressed(gpio_num_t gpio)
{
    return gpio_get_level(gpio) == 0; // Pressionado = nível baixo
}
