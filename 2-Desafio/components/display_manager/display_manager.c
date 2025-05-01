#include "display_manager.h"
#include "ssd1306.h"
#include "ntp_manager.h"
#include "alarm_manager.h"
#include <string.h>
#include <stdio.h>

static SSD1306_t dev;

static char current_time_text[9] = {0};
static bool current_alarm_status = false;
static screen_t current_screen = SCREEN_MAIN;
static screen_t last_screen_displayed = SCREEN_MAIN;
static int menu_index = 0;
static int alarm_list_index = 0;

static bool force_screen_update = true;
static bool force_menu_update = true;
static bool force_alarm_list_update = true;

static int last_displayed_second = -1;

static const char *dias_semana[] = {
    "Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"
};

void display_manager_init(void)
{
    // Inicializa I2C com os pinos definidos no menuconfig
    i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);

    #if CONFIG_SSD1306_128x64
        ssd1306_init(&dev, 128, 64);
    #elif CONFIG_SSD1306_128x32
        ssd1306_init(&dev, 128, 32);
    #else
        #error "Tamanho do display não definido no menuconfig"
    #endif

    #if CONFIG_FLIP
        dev._flip = true;
    #endif

    ssd1306_clear_screen(&dev, false);
}

void display_manager_update(void)
{
    struct tm timeinfo;
    bool time_valid = ntp_manager_get_time(&timeinfo);

    if (current_screen == SCREEN_MAIN) {
        char new_time_text[9];
        char new_weekday_text[16];
        if (time_valid) {
            snprintf(new_time_text, sizeof(new_time_text), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
            snprintf(new_weekday_text, sizeof(new_weekday_text), "Dia: %s", dias_semana[timeinfo.tm_wday]);
        } else {
            snprintf(new_time_text, sizeof(new_time_text), "--:--:--");
            snprintf(new_weekday_text, sizeof(new_weekday_text), "Dia: ---");
        }

        bool alarm_on = alarm_manager_is_enabled();

        if (timeinfo.tm_sec != last_displayed_second || current_alarm_status != alarm_on || force_screen_update) {
            //ssd1306_clear_screen(&dev, false);
            //ssd1306_display_text(&dev, 2, new_time_text, strlen(new_time_text), false);

            ssd1306_clear_line(&dev, 2, false);
            ssd1306_display_text(&dev, 2, new_time_text, strlen(new_time_text), false);

            ssd1306_clear_line(&dev, 4, false);
            const char *status_text = alarm_on ? "Alarmes ON" : "Alarmes OFF";
            ssd1306_display_text(&dev, 4, status_text, strlen(status_text), false);

            ssd1306_clear_line(&dev, 5, false);
            ssd1306_display_text(&dev, 5, new_weekday_text, strlen(new_weekday_text), false);

            strcpy(current_time_text, new_time_text);
            current_alarm_status = alarm_on;
            force_screen_update = false;
            last_screen_displayed = SCREEN_MAIN;
            last_displayed_second = timeinfo.tm_sec;
        }
    }
    else if (current_screen == SCREEN_MENU) {
        if (force_screen_update || force_menu_update) {
            ssd1306_clear_screen(&dev, false);

            if (menu_index == 0) {
                ssd1306_display_text(&dev, 2, "> Lista Alarmes", strlen("> Lista Alarmes"), false);
            } else if (menu_index == 1) {
                ssd1306_display_text(&dev, 2, "> Voltar", strlen("> Voltar"), false);
            }

            force_screen_update = false;
            force_menu_update = false;
            last_screen_displayed = SCREEN_MENU;
        }
    }
    else if (current_screen == SCREEN_ALARMS) {
        if (force_screen_update || force_alarm_list_update) {
            ssd1306_clear_screen(&dev, false);

            alarm_t alarm;
            if (alarm_manager_get_alarm(alarm_list_index, &alarm)) {
                char alarm_text[32];
                snprintf(alarm_text, sizeof(alarm_text), "%02d:%02d dia:%d", alarm.hour, alarm.minute, alarm.weekday);
                ssd1306_display_text(&dev, 2, alarm_text, strlen(alarm_text), false);
            } else {
                ssd1306_display_text(&dev, 2, "Sem alarmes", strlen("Sem alarmes"), false);
            }

            force_screen_update = false;
            force_alarm_list_update = false;
            last_screen_displayed = SCREEN_ALARMS;
        }
    }
    else if (current_screen == SCREEN_EMERGENCY) {
        if (force_screen_update) {
            ssd1306_clear_screen(&dev, true);
            ssd1306_display_text(&dev, 3, "!!! EMERGENCIA !!!", strlen("!!! EMERGENCIA !!!"), true);
            force_screen_update = false;
            last_screen_displayed = SCREEN_EMERGENCY;
        }
    }
}

// Getters e Setters

void display_manager_set_screen(screen_t screen)
{
    current_screen = screen;
    force_screen_update = true;
}

screen_t display_manager_get_screen(void)
{
    return current_screen;
}

void display_manager_next_menu(void)
{
    menu_index = (menu_index + 1) % 2;
    force_menu_update = true;
}

void display_manager_prev_menu(void)
{
    menu_index = (menu_index == 0) ? 1 : (menu_index - 1);
    force_menu_update = true;
}

int display_manager_get_menu_index(void)
{
    return menu_index;
}

void display_manager_next_alarm(void)
{
    if (alarm_list_index < (alarm_manager_count() - 1)) {
        alarm_list_index++;
        force_alarm_list_update = true;
    }
}

void display_manager_prev_alarm(void)
{
    if (alarm_list_index > 0) {
        alarm_list_index--;
        force_alarm_list_update = true;
    }
}
