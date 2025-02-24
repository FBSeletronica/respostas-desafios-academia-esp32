/******************************************************************************
* MQTT Publish Example
* This device publishes temperature and humidity data to a MQTT broker
* and sends an alert if the temperature or humidity is higher than a threshold
*
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
#include "esp_random.h"

#include "wifi.h"                //wifi component
#include "mqtt_app.h"            //mqtt component
#include "dht.h"                 //dht component

//DHT11 configuration
static const dht_sensor_type_t sensor_type = DHT_TYPE_DHT11;
static const gpio_num_t dht_gpio = 15;

//tag for logging
static const char *TAG = "MQTT Publisher";

//thresholds
#define TEMPERATURE_HIGH 30
#define HUMIDITY_HIGH 70

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
    ESP_ERROR_CHECK(wifi_init_sta());                                 //initialize wifi station mode                                                           
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA iniciado...");                     //log wifi station mode started 

    //initialize mqtt
    mqtt_app_start();                                                   //start mqtt client 
    ESP_LOGI(TAG, "MQTT iniciado...");                                  //log mqtt started

    // Wait for the MQTT connection to be established
    if (!xSemaphoreTake(xSemaphoreMqttConnected, pdMS_TO_TICKS(10000)) == pdTRUE)
    {
        ESP_LOGE(TAG, "Failed to connect to MQTT broker");
        return;
    }
    
  
    //auxiliary variables
    int16_t temperature = 0;        //temperature variable
    int16_t humidity = 0;           //humidity variable


    while(1)
    {

        if(dht_read_data(sensor_type,dht_gpio,&humidity,&temperature)==ESP_OK)      //read the data from the sensor
        {
            //LOG 
            ESP_LOGI(TAG, "humidity: %d %%" ,humidity/10);                          //print the humidity   
            ESP_LOGI(TAG, "Temperature: %d C", temperature/10);                     //print the temperature

            char temperature_str[10];                                           //string to store temperature      
            sprintf(temperature_str,"%d",temperature/10);                       //convert temperature to string
            mqtt_app_publish("esp32/temperature",temperature_str,1,1);          //puyblis message "25" to topic "esp32/temperature" with qos 0 and retain 0

            char humidity_str[10];                                              //string to store humidity
            sprintf(humidity_str,"%d",humidity/10);                                //convert humidity to string
            mqtt_app_publish("esp32/humidity",humidity_str,1,1);                //publish message "50" to topic "esp32/humidity" with qos 0 and retain 0

            //check if temperature or humidity is higher than the threshold
            if(temperature/10>TEMPERATURE_HIGH || humidity/10>HUMIDITY_HIGH)                                    
            {
                mqtt_app_publish("esp32/alert","1",1,1);               
            }
            else
            {
                mqtt_app_publish("esp32/alert","0",1,1);                
            }
        }
        else
        {
            ESP_LOGE(TAG, "Could not read data from sensor");                   //print the error message
        }

        vTaskDelay(60000 / portTICK_PERIOD_MS);                             //delay of 60 seconds

    }                                      
}
