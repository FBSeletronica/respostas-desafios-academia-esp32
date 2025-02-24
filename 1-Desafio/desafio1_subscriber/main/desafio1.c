/******************************************************************************
* MQTT Subscriber
* This device subscribe to a topic and control a LED when a message is received

* This example shows is part of a challenge proposed in the course 
* "Academia ESP32 Profissional" by Fábio Souza
* The course is available on https://cursos.embarcados.com.br
*
* This example code Creative Commons Attribution 4.0 International License.
* When using the code, you must keep the above copyright notice,
* this list of conditions and the following disclaimer in the source code.
* (http://creativecommons.org/licenses/by/4.0/)

* Author: Fábio Souza
* This code is for teaching purposes only.
* No warranty of any kind is provided.
*******************************************************************************/
#include <stdio.h>
#include <string.h>

//freertos includes
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "wifi.h"                //wifi component
#include "mqtt_app.h"            //mqtt component

//tag for logging
static const char *TAG = "MQTT Subscriber device";

//LED pin mapping
#define LED_VERMELHO GPIO_NUM_14

//main func
void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {   //if nvs flash is not initialized
      ESP_ERROR_CHECK(nvs_flash_erase());                                             //erase nvs flash
      ret = nvs_flash_init();                                                         //initialize nvs flash
    }
    ESP_ERROR_CHECK(ret);                                                             //check error

    //initialize wifi
    ESP_ERROR_CHECK(wifi_init_sta());
    ESP_LOGI(TAG, "Wi-Fi initialized...");                                         

    //initialize mqtt
    mqtt_app_start();                                                      
    ESP_LOGI(TAG, "MQTT initialized...");                                 

    // Wait for the MQTT connection to be established to subscribe to a topic
    if (xSemaphoreTake(xSemaphoreMqttConnected, pdMS_TO_TICKS(10000)) == pdTRUE) {
      ESP_LOGI(TAG, "MQTT Conected! Subscribing to a topic...");
      mqtt_app_subscribe("esp32/alert", 0);
    } 
    else 
    {
        ESP_LOGE(TAG, "Timeout waiting for MQTT connection");
    }

    //initialize LED
    gpio_reset_pin(LED_VERMELHO);
    gpio_set_direction(LED_VERMELHO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_VERMELHO, 0);

    //process messages
    mqtt_message_t msg;

    while (1) 
    {
      //wait for a message
      if (xQueueReceive(xQueueMqtt, &msg, portMAX_DELAY) == pdTRUE) 
      {
          ESP_LOGI(TAG, "Processing message: [%s] %s", msg.topic, msg.data);

          //check if the message is for the LED
          if (strcmp(msg.topic, "esp32/alert") == 0) 
          {
              int alert = atoi(msg.data);
              gpio_set_level(LED_VERMELHO, alert);
          }
      }
  }                                  
}
