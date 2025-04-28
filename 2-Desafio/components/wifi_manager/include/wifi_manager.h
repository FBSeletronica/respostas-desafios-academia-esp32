#pragma once

#include <stdbool.h> 

typedef void (*wifi_connected_callback_t)(void);

/**
 * @brief Inicializa o Wi-Fi e conecta à rede.
 *
 * @param ssid Nome da rede Wi-Fi (SSID).
 * @param password Senha da rede Wi-Fi.
 * @param cb Callback que será chamado quando a conexão for estabelecida.
 */
void wifi_manager_init(const char* ssid, const char* password, wifi_connected_callback_t cb);

/**
 * @brief Verifica se está conectado ao Wi-Fi.
 *
 * @return true se conectado, false se não conectado.
 */
bool wifi_manager_is_connected(void);

