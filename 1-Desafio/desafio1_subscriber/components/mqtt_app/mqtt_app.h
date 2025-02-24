#ifndef MQTT_APP_H
#define MQTT_APP_H

#include "esp_log.h"
#include "mqtt_client.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#define MQTT_MAX_TOPIC_LEN 100
#define MQTT_MAX_DATA_LEN  256

// Struct to hold a MQTT message
typedef struct {
    char topic[MQTT_MAX_TOPIC_LEN];
    char data[MQTT_MAX_DATA_LEN];
} mqtt_message_t;

void mqtt_app_start(void);
void mqtt_app_subscribe(const char *topic, int qos);
void mqtt_app_unsubscribe(char *topic);
void mqtt_app_publish(const char *topic, const char *payload, int qos, int retain);

// Queue to hold MQTT messages
extern QueueHandle_t xQueueMqtt;

// Semaphore to signal MQTT connection
extern SemaphoreHandle_t xSemaphoreMqttConnected;

#endif