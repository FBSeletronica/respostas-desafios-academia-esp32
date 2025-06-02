#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define MAX_DEVICES 20
#define ESPNOW_PAYLOAD_MAX_SIZE 250
#define MQTT_BASE_TOPIC "home"

typedef enum {
    DEVICE_TYPE_BUTTON = 0,
    DEVICE_TYPE_MOTION_SENSOR = 1,
    DEVICE_TYPE_RELAY = 2,
    DEVICE_TYPE_UNKNOWN = 255
} device_type_t;

typedef struct {
    uint8_t mac[6];
    device_type_t type;
    uint8_t id;
} device_info_t;

typedef struct {
    device_type_t type;
    uint8_t id;
    uint8_t command;
    uint32_t timestamp;
} espnow_message_t;

typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR = 1,
    STATUS_NOT_FOUND = 2
} status_t;

#endif // COMMON_H
