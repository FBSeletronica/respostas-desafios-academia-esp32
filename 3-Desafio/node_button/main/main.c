/*
    Node ESP-NOW que envia mensagens quando o botão é pressionado.
    Este nó deve ser configurado como um dispositivo ESP-NOW no modo STA.
    O nó envia uma mensagem ao gateway quando o botão é pressionado.
    O gateway deve estar configurado para receber mensagens ESP-NOW.

    Certifique-se de que o MAC do gateway esteja correto.

    Autor: Fábio Souza
    Data: 02/06/2025

*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "nvs_flash.h"

#include "common.h"

#define BUTTON_GPIO         2
#define DEBOUNCE_DELAY_MS   50
#define TAG                 "espnow_node_button"

// Atualize com o MAC do gateway (modo STA)
static const uint8_t gateway_mac[6] = { 0x58, 0xBF, 0x25, 0x9F, 0x9E, 0x84 };

// Callback de envio
static void send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    ESP_LOGI("SEND_CB", "Status envio para %02X:%02X:%02X:%02X:%02X:%02X: %s",
             mac_addr[0], mac_addr[1], mac_addr[2],
             mac_addr[3], mac_addr[4], mac_addr[5],
             status == ESP_NOW_SEND_SUCCESS ? "SUCESSO" : "FALHA");
}

void app_main(void)
{
    ESP_LOGI(TAG, "Inicializando nó ESP-NOW (BOTÃO)");

    // Inicializa NVS
    ESP_ERROR_CHECK(nvs_flash_init());

    // Inicializa Wi-Fi em modo station
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE));

    // Inicializa ESP-NOW
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(send_cb));

    // Adiciona o gateway como peer
    esp_now_peer_info_t peer = {0};
    memcpy(peer.peer_addr, gateway_mac, 6);
    peer.channel = 6;  // Definir o mesmo o canal do gateway
    peer.ifidx = ESP_IF_WIFI_STA;
    peer.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&peer));

    bool exists = esp_now_is_peer_exist(gateway_mac);
    ESP_LOGI(TAG, "Peer %s", exists ? "encontrado" : "NÃO encontrado");

    // Configura o botão
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    ESP_LOGI(TAG, "Botão monitorado no GPIO %d", BUTTON_GPIO);

    bool last_state = true;

    while (1) {
        bool current_state = gpio_get_level(BUTTON_GPIO);

        if (last_state && !current_state) {
            ESP_LOGI(TAG, "Botão pressionado!");

            espnow_message_t msg = {
                .type = DEVICE_TYPE_BUTTON,
                .id = 1,
                .command = 1,
                .timestamp = (uint32_t)(esp_timer_get_time() / 1000)
            };

            esp_err_t err = esp_now_send(gateway_mac, (uint8_t*)&msg, sizeof(msg));
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Erro ao enviar ESP-NOW: %s", esp_err_to_name(err));
            }

            // Debounce
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_DELAY_MS));
        }

        last_state = current_state;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
