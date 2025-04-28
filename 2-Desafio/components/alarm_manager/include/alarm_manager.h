#pragma once

#include <stdbool.h>
#include "nvs_storage.h"  // Já tem o alarm_t definido aqui!

// Inicializar o gerenciador de alarmes
void alarm_manager_init(void);

// Checar se o sistema de alarmes está ativado
bool alarm_manager_is_enabled(void);

// Retornar quantos alarmes existem cadastrados
int alarm_manager_count(void);

// Obter um alarme específico por índice
bool alarm_manager_get_alarm(int index, alarm_t *alarm);

// Outras funções futuras...
