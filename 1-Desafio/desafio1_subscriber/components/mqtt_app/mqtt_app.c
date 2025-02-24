#include "mqtt_app.h"

static const char *TAG = "MQTT Client";
static esp_mqtt_client_handle_t client;     //handler for mqtt client
QueueHandle_t xQueueMqtt;                   //message queue for mqtt messages
SemaphoreHandle_t xSemaphoreMqttConnected;  //semaphore to indicate mqtt connection

/*
* Callback function for mqtt events
*/
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) 
    {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "Connected to MQTT Broker");
            
            // Give the semaphore to indicate connection
            if (xSemaphoreMqttConnected != NULL) {
                xSemaphoreGive(xSemaphoreMqttConnected);
            }
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "Desconnected from MQTT Broker");
            
            // Take the semaphore to indicate disconnection
            if (xSemaphoreMqttConnected != NULL) {
                xSemaphoreTake(xSemaphoreMqttConnected, 0);
            }
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "Subscribed to topic, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "Published message, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA: 
        {
            ESP_LOGI(TAG, "Topic: %.*s | Message: %.*s",
                     event->topic_len, event->topic, 
                     event->data_len, event->data);

            // Create a message and send it to the queue    
            mqtt_message_t msg;
            snprintf(msg.topic, sizeof(msg.topic), "%.*s", event->topic_len, event->topic);
            snprintf(msg.data, sizeof(msg.data), "%.*s", event->data_len, event->data);

            // Send the message to the queue
            if (xQueueMqtt != NULL) 
            {
                if (xQueueSend(xQueueMqtt, &msg, portMAX_DELAY) != pdPASS) {
                    ESP_LOGW(TAG, "Failed to send message to queue");
                }
            }
            break;
        }
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Another Event: %d", event->event_id);
            break;
    }
}

void mqtt_app_start(void)
{
    // Create the queue for MQTT messages
    xQueueMqtt = xQueueCreate(10, sizeof(mqtt_message_t));
    if (xQueueMqtt == NULL) {
        ESP_LOGE(TAG, "Error creating the MQTT queue!");
        return;
    }

    // Create the semaphore for MQTT connection
    xSemaphoreMqttConnected = xSemaphoreCreateBinary();
    if (xSemaphoreMqttConnected == NULL) {
        ESP_LOGE(TAG, "Error creating the MQTT semaphore!");
        return;
    }

    // Initialize the MQTT client
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_BROKER_URL,
        .broker.address.port = 1883,
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void mqtt_app_subscribe(const char *topic, int qos) {
    int msg_id = esp_mqtt_client_subscribe(client, topic, qos);
    ESP_LOGI(TAG, "Sent Subscribe, msg_id=%d", msg_id);
}

void mqtt_app_unsubscribe(char *topic)
{
    int msg_id = esp_mqtt_client_unsubscribe(client, topic);
    ESP_LOGI(TAG, "Sent unsubscribe successful, msg_id=%d", msg_id);
}

// Publica uma mensagem MQTT
void mqtt_app_publish(const char *topic, const char *payload, int qos, int retain) {
    int msg_id = esp_mqtt_client_publish(client, topic, payload, strlen(payload), qos, retain);
    ESP_LOGI(TAG, "Sent Message, msg_id=%d", msg_id);
}

