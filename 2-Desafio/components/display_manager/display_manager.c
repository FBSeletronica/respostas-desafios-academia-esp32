#include "display_manager.h"
#include "ssd1306.h"
#include "ntp_manager.h"
#include "alarm_manager.h"
#include "buttons.h"
#include "buzzer.h"
#include <string.h>
#include <stdio.h>

static SSD1306_t dev;


static char current_time_text[9] = {0};
static bool current_alarm_status = false;
static screen_t current_screen = SCREEN_MAIN;
static screen_t last_screen_displayed = SCREEN_MAIN;
static int menu_index = 0;
static int last_menu_index = -1;
static int alarm_list_index = 0;
static int last_alarm_list_index = -1;

static bool force_screen_update = true;
static bool force_menu_update = true;
static bool force_alarm_list_update = true;

void display_manager_init(void)
{
    ssd1306_clear_screen(&dev, false);
}

void display_manager_update(void)
{
    struct tm timeinfo;
    bool time_valid = ntp_manager_get_time(&timeinfo);

    if (current_screen == SCREEN_MAIN) {
        char new_time_text[9];
        if (time_valid) {
            snprintf(new_time_text, sizeof(new_time_text), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        } else {
            snprintf(new_time_text, sizeof(new_time_text), "--:--:--");
        }

        bool alarm_on = alarm_manager_is_enabled();

        if (strcmp(current_time_text, new_time_text) != 0 || current_alarm_status != alarm_on || force_screen_update) {
            ssd1306_clear_screen(&dev, false);
            ssd1306_display_text(&dev, 2, new_time_text, strlen(new_time_text), false);

            const char *status_text = alarm_on ? "Alarmes ON" : "Alarmes OFF";
            ssd1306_display_text(&dev, 4, status_text, strlen(status_text), false);

            strcpy(current_time_text, new_time_text);
            current_alarm_status = alarm_on;
            force_screen_update = false;
            last_screen_displayed = SCREEN_MAIN;
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

void display_manager_handle_buttons(void)
{
    static bool emergency_pressed = false;

    if (button_emergency_pressed() && !emergency_pressed) {
        emergency_pressed = true;
        current_screen = SCREEN_EMERGENCY;
        buzzer_start_melody(BUZZER_MELODY_EMERGENCY, 10000);
        force_screen_update = true;
    }
    if (!button_emergency_pressed()) {
        emergency_pressed = false;
    }

    switch (current_screen)
    {
        case SCREEN_MAIN:
            if (button_enter_pressed()) {
                buzzer_start_melody(BUZZER_MELODY_NORMAL, 100);
                current_screen = SCREEN_MENU;
                force_screen_update = true;
            }
            break;

        case SCREEN_MENU:
            if (button_down_pressed()) {
                buzzer_start_melody(BUZZER_MELODY_NORMAL, 100);
                menu_index = (menu_index + 1) % 2;
                force_menu_update = true;
            }

            if (button_up_pressed()) {
                buzzer_start_melody(BUZZER_MELODY_NORMAL, 100);
                menu_index = (menu_index == 0) ? 1 : (menu_index - 1);
                force_menu_update = true;
            }

            if (button_enter_pressed()) {
                buzzer_start_melody(BUZZER_MELODY_NORMAL, 100);
                if (menu_index == 0) {
                    current_screen = SCREEN_ALARMS;
                    alarm_list_index = 0;
                } else {
                    current_screen = SCREEN_MAIN;
                }
                force_screen_update = true;
            }

            if (button_left_pressed()) {
                buzzer_start_melody(BUZZER_MELODY_NORMAL, 100);
                current_screen = SCREEN_MAIN;
                force_screen_update = true;
            }
            break;

        case SCREEN_ALARMS:
            if (button_down_pressed()) {
                buzzer_start_melody(BUZZER_MELODY_NORMAL, 100);
                if (alarm_list_index < (alarm_manager_count() - 1)) {
                    alarm_list_index++;
                    force_alarm_list_update = true;
                }
            }

            if (button_up_pressed()) {
                buzzer_start_melody(BUZZER_MELODY_NORMAL, 100);
                if (alarm_list_index > 0) {
                    alarm_list_index--;
                    force_alarm_list_update = true;
                }
            }

            if (button_left_pressed() || button_enter_pressed()) {
                buzzer_start_melody(BUZZER_MELODY_NORMAL, 100);
                current_screen = SCREEN_MENU;
                force_screen_update = true;
            }
            break;

        case SCREEN_EMERGENCY:
            if (button_enter_pressed() || button_left_pressed()) {
                buzzer_start_melody(BUZZER_MELODY_NORMAL, 100);
                current_screen = SCREEN_MAIN;
                buzzer_stop();
                force_screen_update = true;
            }
            break;

        default:
            break;
    }
}
