/*
    Gateway principal para gerenciar dispositivos IoT via ESP-NOW e MQTT.a64l

    Este código é responsável por receber mensagens de dispositivos via ESP-NOW,
    registrar novos dispositivos, e encaminhar comandos via MQTT para dispositivos controláveis.

    Autor: Fábio Souza
    Data: 02/06/2025
*/

#include <string.h>
#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "espnow_manager.h"
#include "device_registry.h" 
#include "common.h"
#include "esp_mac.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_private/wifi.h"
#include "esp_log.h"
#include "cJSON.h"

static const char *TAG = "gateway_main";

// callback para receber mensagens ESP-NOW
static void on_espnow_message_received(const uint8_t *mac_addr, const espnow_message_t *message)
{
    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac_addr[0], mac_addr[1], mac_addr[2],
             mac_addr[3], mac_addr[4], mac_addr[5]);

    device_info_t device;

    if (device_registry_find(mac_addr, &device) != STATUS_OK) {
        ESP_LOGW(TAG, "Dispositivo desconhecido: %s. Registrando automaticamente.", mac_str);

        device_info_t new_device = {
            .type = message->type,
            .id = message->id
        };
        memcpy(new_device.mac, mac_addr, 6);

        if (device_registry_add(&new_device) == STATUS_OK) {
            device = new_device;
            ESP_LOGI(TAG, "Dispositivo registrado com sucesso: tipo=%d, id=%d", device.type, device.id);

            // Assina o tópico se o dispositivo for controlável
            if (device.type == DEVICE_TYPE_RELAY) {
                char topic[64];
                snprintf(topic, sizeof(topic), "%s/%d/%d/set", MQTT_BASE_TOPIC, device.type, device.id);
                mqtt_manager_subscribe(topic);
            }

            // Adiciona como peer ESP-NOW
            if (!esp_now_is_peer_exist(mac_addr)) {
                esp_now_peer_info_t peer = {0};
                memcpy(peer.peer_addr, mac_addr, 6);
                peer.channel = 6;  // mesmo canal do gateway
                peer.ifidx = ESP_IF_WIFI_STA;
                peer.encrypt = false;

                esp_err_t err = esp_now_add_peer(&peer);
                if (err == ESP_OK) {
                    ESP_LOGI(TAG, "Peer adicionado com sucesso: %s", mac_str);
                } else {
                    ESP_LOGE(TAG, "Falha ao adicionar peer: %s (%s)", mac_str, esp_err_to_name(err));
                }
            }
        } else {
            ESP_LOGE(TAG, "Falha ao registrar novo dispositivo");
            return;
        }
    }

    // Monta o tópico e a mensagem MQTT
    char topic[64];
    snprintf(topic, sizeof(topic), "%s/%d/%d/event", MQTT_BASE_TOPIC, device.type, device.id);

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "mac", mac_str);
    cJSON_AddNumberToObject(root, "command", message->command);
    cJSON_AddNumberToObject(root, "timestamp", message->timestamp);

    char *payload = cJSON_PrintUnformatted(root);
    mqtt_manager_publish(topic, payload);

    ESP_LOGI(TAG, "Publicou evento: %s -> %s", topic, payload);

    cJSON_Delete(root);
    free(payload);
}

// Callback para receber mensagens MQTT
static void on_mqtt_message_received(const char *topic, const char *payload)
{
    ESP_LOGI(TAG, "Comando MQTT recebido: %s -> %s", topic, payload);

    // Aqui o formato esperado é home/<device_type>/<device_id>/set
    int device_type, device_id;
    if (sscanf(topic, MQTT_BASE_TOPIC"/%d/%d/set", &device_type, &device_id) != 2) {
        ESP_LOGW(TAG, "Tópico MQTT não reconhecido: %s", topic);
        return;
    }

    // Buscar device_info
    device_info_t device;
    if (device_registry_find_by_type_id(device_type, device_id, &device) != STATUS_OK) {
        ESP_LOGW(TAG, "Dispositivo não encontrado para comando MQTT");
        return;
    }

    // Parsear o payload (esperado: {"command": 0/1})
    cJSON *root = cJSON_Parse(payload);
    if (!root) {
        ESP_LOGW(TAG, "Falha ao parsear payload MQTT");
        return;
    }

    cJSON *cmd = cJSON_GetObjectItem(root, "command");
    if (!cJSON_IsNumber(cmd)) {
        ESP_LOGW(TAG, "Campo 'command' inválido no payload");
        cJSON_Delete(root);
        return;
    }

    espnow_message_t message = {
        .type = device.type,
        .id = device.id,
        .command = (uint8_t)cmd->valueint,
        .timestamp = 0 
    };

    espnow_manager_send(device.mac, &message);

    cJSON_Delete(root);
}

// Função principal
void app_main(void)
{
    ESP_LOGI(TAG, "Inicializando Gateway...");

    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    ESP_LOGI(TAG, "MAC do gateway: %02X:%02X:%02X:%02X:%02X:%02X", 
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);


    // Inicializar Wi-Fi
    wifi_manager_init();

    // Imprime o MAC do gateway
    uint8_t primary;
    wifi_second_chan_t second;
    esp_wifi_get_channel(&primary, &second);
    ESP_LOGI(TAG, "Canal Wi-Fi do gateway: %d", primary);


    // Inicializar MQTT
    mqtt_manager_init();
    mqtt_manager_set_callback(on_mqtt_message_received);

    // Inicializar ESP-NOW
    espnow_manager_init();
    espnow_manager_set_receive_callback(on_espnow_message_received);

    // Inicializar Registro de Dispositivos
    device_registry_init();

    // Sinaliza que o gateway está pronto
    ESP_LOGI(TAG, "Gateway pronto!");
}
