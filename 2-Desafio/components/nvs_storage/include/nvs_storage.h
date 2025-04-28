#pragma once

#include <stdint.h>
#include <stdbool.h>

#define MAX_ALARMS 20  // Número máximo de alarmes armazenados

typedef struct {
    int hour;
    int minute;
    int weekday;
    int melody;
} alarm_t;

/**
 * @brief Inicializa o sistema de armazenamento NVS.
 */
void nvs_storage_init(void);

/**
 * @brief Salva um alarme na memória.
 *
 * @param alarm Alarme a ser salvo.
 * @return true se salvo com sucesso, false em caso de erro.
 */
bool nvs_storage_save_alarm(const alarm_t *alarm);

/**
 * @brief Carrega todos os alarmes salvos.
 *
 * @param alarms Array onde os alarmes serão armazenados.
 * @param max_alarms Capacidade máxima do array.
 * @return Número de alarmes carregados.
 */
int nvs_storage_load_alarms(alarm_t *alarms, int max_alarms);

/**
 * @brief Limpa todos os alarmes da memória.
 *
 * @return true se limpar com sucesso, false se erro.
 */
bool nvs_storage_clear_alarms(void);
