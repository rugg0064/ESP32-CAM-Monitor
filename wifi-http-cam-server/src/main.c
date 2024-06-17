// // Camera
#include "camera/camera.h"
#include "esp_camera.h"

// // Wifi
#include "wifi/wifi.h"

// // Http server
#include "http/http.h"
#include "esp_http_server.h"

// Extras
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "MAIN";

// Handler function for HTTP GET query
esp_err_t get_capture_handler(httpd_req_t *req)
{
    camera_fb_t *pic = esp_camera_fb_get();
    if (!pic) {
        ESP_LOGE(TAG, "Failed to get camera frame buffer");
        // Failed to get the camera frame buffer
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_send(req, (const char *)pic->buf, pic->len);
    esp_camera_fb_return(pic);
    return ESP_OK;
}

httpd_uri_t uri_get_capture = {
    .uri      = "/capture",
    .method   = HTTP_GET,
    .handler  = get_capture_handler,
    .user_ctx = NULL
};


void app_main() {
    ESP_ERROR_CHECK(setup_wifi());
    ESP_ERROR_CHECK(init_camera());

    httpd_handle_t webServer = start_webserver();
    if (webServer == NULL) {
        ESP_LOGE(TAG, "Failed to start web server");
        return;
    }
    ESP_LOGI(TAG, "Web server started");

    httpd_register_uri_handler(webServer, &uri_get_capture);

    while(1) {
        ESP_LOGI(TAG, "WAITING");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}