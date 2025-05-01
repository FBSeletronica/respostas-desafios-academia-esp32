#pragma once

#include <stdbool.h>

// Enum de telas
typedef enum {
    SCREEN_MAIN,
    SCREEN_MENU,
    SCREEN_ALARMS,
    SCREEN_EMERGENCY
} screen_t;

void display_manager_init(void);
void display_manager_update(void);

// Controle de tela
void display_manager_set_screen(screen_t screen);
screen_t display_manager_get_screen(void);

// Navegação de menu
void display_manager_next_menu(void);
void display_manager_prev_menu(void);
int display_manager_get_menu_index(void);

// Navegação de alarmes
void display_manager_next_alarm(void);
void display_manager_prev_alarm(void);
