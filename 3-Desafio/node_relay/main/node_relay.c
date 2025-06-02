/*
* ESP-NOW Relay Node
* Este código implementa um nó ESP-NOW que atua como um relé controlado por comandos recebidos via ESP-NOW.
* O relé é conectado ao GPIO 14 e pode ser ligado ou desligado por comandos enviados de um gateway.
* O nó também envia uma mensagem de presença ao gateway para indicar que está ativo.

* Autor: Fábio Souza  
* Data:  02/06/2025
*/

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_timer.h"

#include "common.h"

#define RELAY_GPIO 14
#define TAG "espnow_node_relay"

// MAC do gateway
static const uint8_t gateway_mac[6] = { 0x58, 0xBF, 0x25, 0x9F, 0x9E, 0x84 };

// Callback para pacotes recebidos
static void on_espnow_recv(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    if (!recv_info || !data || len != sizeof(espnow_message_t)) {
        ESP_LOGW(TAG, "Pacote inválido recebido");
        return;
    }

    const espnow_message_t *msg = (const espnow_message_t *)data;

    ESP_LOGI(TAG, "Comando recebido: tipo=%d id=%d cmd=%d", msg->type, msg->id, msg->command);

    if (msg->type == DEVICE_TYPE_RELAY && msg->id == 1) {
        gpio_set_level(RELAY_GPIO, msg->command ? 1 : 0);
        ESP_LOGI(TAG, "Relé %s", msg->command ? "LIGADO" : "DESLIGADO");
    }
}

// Função principal 
void app_main(void)
{
    ESP_LOGI(TAG, "Inicializando nó ESP-NOW (RELÉ)");

    ESP_ERROR_CHECK(nvs_flash_init());

    // Wi-Fi STA + fixar canal do gateway
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // Fixar canal igual ao do gateway
    ESP_ERROR_CHECK(esp_wifi_set_channel(6, WIFI_SECOND_CHAN_NONE)); // ajuste conforme necessário

    // Inicializa ESP-NOW
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_espnow_recv));

    // Adiciona o gateway como peer 
    esp_now_peer_info_t peer = {0};
    memcpy(peer.peer_addr, gateway_mac, 6);
    peer.channel = 6;
    peer.ifidx = ESP_IF_WIFI_STA;
    peer.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&peer));

    // Configura GPIO do relé
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RELAY_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(RELAY_GPIO, 0); // desligado inicialmente

    // Envia mensagem de presença para o gateway
    espnow_message_t presence = {
        .type = DEVICE_TYPE_RELAY,
        .id = 1,
        .command = 2, // presença
        .timestamp = (uint32_t)(esp_timer_get_time() / 1000)
    };

    esp_err_t err = esp_now_send(gateway_mac, (uint8_t*)&presence, sizeof(presence));
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "Mensagem de presença enviada ao gateway");
    } else {
        ESP_LOGE(TAG, "Erro ao enviar mensagem de presença: %s", esp_err_to_name(err));
    }

    // Sinaliza que o relé está pronto
    ESP_LOGI(TAG, "Relé pronto no GPIO %d", RELAY_GPIO);

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
