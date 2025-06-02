#include "mqtt_manager.h"
#include "wifi_manager.h"

#include "esp_log.h"
#include "mqtt_client.h"

static const char *TAG = "mqtt_manager";
static esp_mqtt_client_handle_t client = NULL;
static mqtt_message_callback_t app_message_callback = NULL;

// Handler de eventos MQTT
static void mqtt_event_handler_cb(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Conectado ao broker MQTT.");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "Desconectado do broker MQTT.");
            break;
        case MQTT_EVENT_DATA:
            if (app_message_callback) {
                char topic[event->topic_len + 1];
                char data[event->data_len + 1];
                memcpy(topic, event->topic, event->topic_len);
                topic[event->topic_len] = '\0';
                memcpy(data, event->data, event->data_len);
                data[event->data_len] = '\0';
                app_message_callback(topic, data);
            }
            break;
        default:
            break;
    }
}

// Inicializa o cliente MQTT
esp_err_t mqtt_manager_init(void)
{
    if (!wifi_manager_is_connected()) {
        ESP_LOGW(TAG, "Wi-Fi ainda não conectado. Aguardando...");
        while (!wifi_manager_is_connected()) {
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }

    const esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.uri = "mqtt://broker.hivemq.com"    // trocar pelo broker desejado
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    if (client == NULL) {
        ESP_LOGE(TAG, "Falha ao inicializar MQTT Client");
        return ESP_FAIL;
    }

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler_cb, NULL);
    esp_mqtt_client_start(client);

    return ESP_OK;
}

// Publica uma mensagem
esp_err_t mqtt_manager_publish(const char* topic, const char* payload)
{
    if (client == NULL) {
        ESP_LOGW(TAG, "Cliente MQTT não iniciado");
        return ESP_FAIL;
    }

    int msg_id = esp_mqtt_client_publish(client, topic, payload, 0, 1, 0);
    ESP_LOGI(TAG, "Mensagem publicada no tópico %s: %s", topic, payload);
    return (msg_id >= 0) ? ESP_OK : ESP_FAIL;
}

// Registra um callback de mensagem recebida
void mqtt_manager_set_callback(mqtt_message_callback_t callback)
{
    app_message_callback = callback;
}

// Inscreve-se em um tópico MQTT
esp_err_t mqtt_manager_subscribe(const char* topic)
{
    if (client == NULL) return ESP_FAIL;
    int msg_id = esp_mqtt_client_subscribe(client, topic, 1);
    ESP_LOGI(TAG, "Assinando tópico: %s (msg_id=%d)", topic, msg_id);
    return (msg_id >= 0) ? ESP_OK : ESP_FAIL;
}

