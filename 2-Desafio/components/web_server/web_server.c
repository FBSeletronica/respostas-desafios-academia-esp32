#include "web_server.h"
#include "nvs_storage.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "cJSON.h"
#include <string.h>

static const char *TAG = "WEB_SERVER";
static httpd_handle_t server = NULL;

static esp_err_t get_alarms_handler(httpd_req_t *req)
{
    alarm_t alarms[MAX_ALARMS];
    int num_alarms = nvs_storage_load_alarms(alarms, MAX_ALARMS);

    cJSON *root = cJSON_CreateArray();
    for (int i = 0; i < num_alarms; i++) {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "hour", alarms[i].hour);
        cJSON_AddNumberToObject(item, "minute", alarms[i].minute);
        cJSON_AddNumberToObject(item, "weekday", alarms[i].weekday);
        cJSON_AddNumberToObject(item, "melody", alarms[i].melody);
        cJSON_AddItemToArray(root, item);
    }

    const char *response = cJSON_Print(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, response);

    cJSON_Delete(root);
    free((void *)response);

    return ESP_OK;
}

static esp_err_t post_alarm_handler(httpd_req_t *req)
{
    char buf[200];
    int ret = httpd_req_recv(req, buf, sizeof(buf));
    if (ret <= 0) {
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    cJSON *root = cJSON_Parse(buf);
    if (!root) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "JSON inválido");
        return ESP_FAIL;
    }

    alarm_t alarm;
    alarm.hour = cJSON_GetObjectItem(root, "hour")->valueint;
    alarm.minute = cJSON_GetObjectItem(root, "minute")->valueint;
    alarm.weekday = cJSON_GetObjectItem(root, "weekday")->valueint;
    alarm.melody = cJSON_GetObjectItem(root, "melody")->valueint;

    if (nvs_storage_save_alarm(&alarm)) {
        httpd_resp_sendstr(req, "Alarme salvo com sucesso!");
    } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Falha ao salvar alarme");
    }

    cJSON_Delete(root);
    return ESP_OK;
}

static esp_err_t delete_alarms_handler(httpd_req_t *req)
{
    if (nvs_storage_clear_alarms()) {
        httpd_resp_sendstr(req, "Alarmes apagados com sucesso!");
    } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Falha ao apagar alarmes");
    }
    return ESP_OK;
}

static esp_err_t root_get_handler(httpd_req_t *req)
{
    FILE* f = fopen("/spiffs/alarm_manager.html", "r");
    if (f == NULL) {
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "text/html");

    char buffer[512];
    size_t read_bytes;
    while ((read_bytes = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        httpd_resp_send_chunk(req, buffer, read_bytes);
    }
    fclose(f);
    httpd_resp_send_chunk(req, NULL, 0); // Indicar fim da resposta
    return ESP_OK;
}

void web_server_start(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    ESP_LOGI(TAG, "Iniciando Web Server na porta %d...", config.server_port);

    if (httpd_start(&server, &config) == ESP_OK) {

        // GET /
        httpd_uri_t root_uri = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = root_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &root_uri);

        // GET /alarms
        httpd_uri_t get_alarms_uri = {
            .uri       = "/alarms",
            .method    = HTTP_GET,
            .handler   = get_alarms_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &get_alarms_uri);

        // POST /alarms
        httpd_uri_t post_alarm_uri = {
            .uri       = "/alarms",
            .method    = HTTP_POST,
            .handler   = post_alarm_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &post_alarm_uri);

        // DELETE /alarms
        httpd_uri_t delete_alarms_uri = {
            .uri       = "/alarms",
            .method    = HTTP_DELETE,
            .handler   = delete_alarms_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &delete_alarms_uri);

    } else {
        ESP_LOGE(TAG, "Falha ao iniciar Web Server");
    }
}


