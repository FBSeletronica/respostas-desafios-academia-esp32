#include "espnow_manager.h"
#include "common.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "espnow_manager";
static espnow_receive_callback_t app_receive_callback = NULL;

// Callback interno para recepção ESP-NOW
static void espnow_recv_cb(const esp_now_recv_info_t *recv_info, const uint8_t *data, int len)
{
    if (!recv_info || !data || len <= 0) {
        ESP_LOGW(TAG, "Pacote ESP-NOW inválido recebido");
        return;
    }

    const uint8_t *mac_addr = recv_info->src_addr;

    if (len != sizeof(espnow_message_t)) {
        ESP_LOGW(TAG, "Tamanho inesperado de mensagem: %d", len);
        return;
    }

    espnow_message_t message;
    memcpy(&message, data, sizeof(espnow_message_t));

    ESP_LOGI(TAG, "Recebido de: %02X:%02X:%02X:%02X:%02X:%02X | Tipo: %d | ID: %d | Cmd: %d",
             mac_addr[0], mac_addr[1], mac_addr[2],
             mac_addr[3], mac_addr[4], mac_addr[5],
             message.type, message.id, message.command);

    if (app_receive_callback) {
        app_receive_callback(mac_addr, &message);
    }
}


// Callback interno para confirmação de envio ESP-NOW
static void espnow_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    ESP_LOGI(TAG, "Envio para %02X:%02X:%02X:%02X:%02X:%02X -> %s",
             mac_addr[0], mac_addr[1], mac_addr[2],
             mac_addr[3], mac_addr[4], mac_addr[5],
             status == ESP_NOW_SEND_SUCCESS ? "Sucesso" : "Falha");
}

// Inicializa ESP-NOW
esp_err_t espnow_manager_init(void)
{
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_recv_cb));
    ESP_ERROR_CHECK(esp_now_register_send_cb(espnow_send_cb));

    ESP_LOGI(TAG, "ESP-NOW inicializado.");

    return ESP_OK;
}

// Envia mensagem ESP-NOW para um nó
esp_err_t espnow_manager_send(const uint8_t *mac_addr, const espnow_message_t *message)
{
    if (!mac_addr || !message) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = esp_now_send(mac_addr, (uint8_t *)message, sizeof(espnow_message_t));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao enviar pacote ESP-NOW: %s", esp_err_to_name(err));
        ESP_LOGE(TAG, "Falha ao enviar pacote ESP-NOW");
    }
    return err;
}

// Permite registrar o callback de recebimento
void espnow_manager_set_receive_callback(espnow_receive_callback_t callback)
{
    app_receive_callback = callback;
}
