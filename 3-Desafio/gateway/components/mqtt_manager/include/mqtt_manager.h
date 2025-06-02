#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include "esp_err.h"
#include "common.h"

// Inicializa o cliente MQTT
esp_err_t mqtt_manager_init(void);

// Publica uma mensagem em um tópico
esp_err_t mqtt_manager_publish(const char* topic, const char* payload);

// Função para registrar callback de mensagem recebida
typedef void (*mqtt_message_callback_t)(const char* topic, const char* payload);

// Registra a função de callback para mensagens MQTT recebidas
void mqtt_manager_set_callback(mqtt_message_callback_t callback);

// Inscreve-se em um tópico MQTT
esp_err_t mqtt_manager_subscribe(const char* topic);

#endif // MQTT_MANAGER_H
