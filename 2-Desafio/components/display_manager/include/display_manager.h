#pragma once

typedef enum {
    SCREEN_MAIN,
    SCREEN_MENU,
    SCREEN_ALARMS,
    SCREEN_EMERGENCY
} screen_t;

void display_manager_init(void);
void display_manager_update(void);
void display_manager_handle_buttons(void);

void display_manager_set_screen(screen_t screen);
screen_t display_manager_get_screen(void);

void display_manager_next_menu(void);
void display_manager_prev_menu(void);

void display_manager_next_alarm(void);
void display_manager_prev_alarm(void);

void display_manager_reset_navigation(void);
