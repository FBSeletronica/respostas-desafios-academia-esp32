#include "display_manager.h"
#include "ssd1306.h"
#include "ntp_manager.h"
#include "alarm_manager.h"
#include "buttons.h"
#include "buzzer.h"
#include <string.h>
#include <stdio.h>

static SSD1306_t dev;

// Estado atual da aplicação
typedef enum {
    SCREEN_MAIN,
    SCREEN_MENU,
    SCREEN_ALARMS,
    SCREEN_EMERGENCY
} screen_t;

// Controle de atualização para reduzir flicker
static char current_time_text[9] = {0};
static bool current_alarm_status = false;
static screen_t current_screen = SCREEN_MAIN;
static screen_t last_screen_displayed = SCREEN_MAIN;
static int menu_index = 0;
static int last_menu_index = -1;
static int alarm_list_index = 0;
static int last_alarm_list_index = -1;

void display_manager_init(void)
{
    // A inicialização do display foi feita externamente
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

        if (strcmp(current_time_text, new_time_text) != 0 || current_alarm_status != alarm_on || last_screen_displayed != SCREEN_MAIN) {
            ssd1306_clear_screen(&dev, false);
            ssd1306_display_text(&dev, 2, new_time_text, strlen(new_time_text), false);

            const char *status_text = alarm_on ? "Alarmes ON" : "Alarmes OFF";
            ssd1306_display_text(&dev, 4, status_text, strlen(status_text), false);

            strcpy(current_time_text, new_time_text);
            current_alarm_status = alarm_on;
            last_screen_displayed = SCREEN_MAIN;
        }
    }
    else if (current_screen == SCREEN_MENU) {
        if (last_screen_displayed != SCREEN_MENU || last_menu_index != menu_index) {
            ssd1306_clear_screen(&dev, false);

            if (menu_index == 0) {
                ssd1306_display_text(&dev, 2, "> Lista Alarmes", strlen("> Lista Alarmes"), false);
            } else if (menu_index == 1) {
                ssd1306_display_text(&dev, 2, "> Voltar", strlen("> Voltar"), false);
            }

            last_screen_displayed = SCREEN_MENU;
            last_menu_index = menu_index;
        }
    }
    else if (current_screen == SCREEN_ALARMS) {
        if (last_screen_displayed != SCREEN_ALARMS || last_alarm_list_index != alarm_list_index) {
            ssd1306_clear_screen(&dev, false);

            alarm_t alarm;
            if (alarm_manager_get_alarm(alarm_list_index, &alarm)) {
                char alarm_text[32];
                snprintf(alarm_text, sizeof(alarm_text), "%02d:%02d dia:%d", alarm.hour, alarm.minute, alarm.weekday);
                ssd1306_display_text(&dev, 2, alarm_text, strlen(alarm_text), false);
            } else {
                ssd1306_display_text(&dev, 2, "Sem alarmes", strlen("Sem alarmes"), false);
            }

            last_screen_displayed = SCREEN_ALARMS;
            last_alarm_list_index = alarm_list_index;
        }
    }
    else if (current_screen == SCREEN_EMERGENCY) {
        if (last_screen_displayed != SCREEN_EMERGENCY) {
            ssd1306_clear_screen(&dev, true);
            ssd1306_display_text(&dev, 3, "!!! EMERGENCIA !!!", strlen("!!! EMERGENCIA !!!"), true);
            last_screen_displayed = SCREEN_EMERGENCY;
        }
    }
}

void display_manager_set_screen(screen_t screen)
{
    current_screen = screen;
    last_screen_displayed = -1; // Força redesenho completo
}

screen_t display_manager_get_screen(void)
{
    return current_screen;
}

void display_manager_next_menu(void)
{
    menu_index = (menu_index + 1) % 2;
}

void display_manager_prev_menu(void)
{
    menu_index = (menu_index == 0) ? 1 : (menu_index - 1);
}

void display_manager_next_alarm(void)
{
    if (alarm_list_index < (alarm_manager_count() - 1)) {
        alarm_list_index++;
    }
}

void display_manager_prev_alarm(void)
{
    if (alarm_list_index > 0) {
        alarm_list_index--;
    }
}

void display_manager_reset_navigation(void)
{
    menu_index = 0;
    alarm_list_index = 0;
}


void display_manager_handle_buttons(void)
{
    if (button_emergency_pressed()) {
        current_screen = SCREEN_EMERGENCY;
        last_screen_displayed = -1; // Força atualização!
        buzzer_start_melody(BUZZER_MELODY_EMERGENCY, 10000);
        return;
    }

    switch (current_screen)
    {
        case SCREEN_MAIN:
            if (button_enter_pressed()) {
                current_screen = SCREEN_MENU;
                last_screen_displayed = -1; // Força redesenhar
            }
            break;

        case SCREEN_MENU:
            if (button_down_pressed()) {
                menu_index = (menu_index + 1) % 2;
                last_menu_index = -1; // Força redesenhar opção
            }

            if (button_up_pressed()) {
                menu_index = (menu_index == 0) ? 1 : (menu_index - 1);
                last_menu_index = -1; // Força redesenhar opção
            }

            if (button_enter_pressed()) {
                if (menu_index == 0) {
                    current_screen = SCREEN_ALARMS;
                    alarm_list_index = 0;
                    last_screen_displayed = -1; // Força redesenhar
                }
                else if (menu_index == 1) {
                    current_screen = SCREEN_MAIN;
                    last_screen_displayed = -1; // Força redesenhar
                }
            }

            if (button_left_pressed()) {
                current_screen = SCREEN_MAIN;
                last_screen_displayed = -1;
            }
            break;

        case SCREEN_ALARMS:
            if (button_down_pressed()) {
                if (alarm_list_index < (alarm_manager_count() - 1)) {
                    alarm_list_index++;
                    last_alarm_list_index = -1;
                }
            }

            if (button_up_pressed()) {
                if (alarm_list_index > 0) {
                    alarm_list_index--;
                    last_alarm_list_index = -1;
                }
            }

            if (button_left_pressed() || button_enter_pressed()) {
                current_screen = SCREEN_MENU;
                last_screen_displayed = -1;
            }
            break;

        case SCREEN_EMERGENCY:
            if (button_enter_pressed() || button_left_pressed()) {
                current_screen = SCREEN_MAIN;
                last_screen_displayed = -1;
                buzzer_stop();
            }
            break;

        default:
            break;
    }
}


