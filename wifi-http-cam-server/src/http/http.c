#include "http.h"
#include "esp_log.h"
#include "esp_http_server.h"

// static const char *TAG = "HTTP";

// Handler function for HTTP GET query
esp_err_t get_status_handler(httpd_req_t *req)
{
    /* Send a simple response */
    const char resp[] = "Status is healthy";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

httpd_uri_t uri_get = {
    .uri      = "/status",
    .method   = HTTP_GET,
    .handler  = get_status_handler,
    .user_ctx = NULL
};

// Start webserver (https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/protocols/esp_http_server.html)
// Returns NULL if failed
httpd_handle_t start_webserver(void)
{
    /* Generate default configuration */
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Register URI handlers */
        httpd_register_uri_handler(server, &uri_get);
        // httpd_register_uri_handler(server, &uri_post); // Not using post
    }
    /* If server failed to start, handle will be NULL */
    return server;
}