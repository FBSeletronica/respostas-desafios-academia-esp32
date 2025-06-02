#ifndef ESPNOW_MANAGER_H
#define ESPNOW_MANAGER_H

#include "esp_err.h"
#include "common.h"

// Inicializa ESP-NOW
esp_err_t espnow_manager_init(void);

// Envia mensagem para um nó
esp_err_t espnow_manager_send(const uint8_t *mac_addr, const espnow_message_t *message);

// Registra callback para recebimento de mensagens
typedef void (*espnow_receive_callback_t)(const uint8_t *mac_addr, const espnow_message_t *message);

// Define a função de callback para receber mensagens ESP-NOW
void espnow_manager_set_receive_callback(espnow_receive_callback_t callback);

#endif // ESPNOW_MANAGER_H
