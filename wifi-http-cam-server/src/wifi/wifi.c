#include "wifi.h"
#include "esp_log.h"

#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"

static const char *TAG = "WIFI";

// Event handler for Wi-Fi events
static void wifi_event_handler(
    void* arg, 
    esp_event_base_t event_base,
    int32_t event_id, 
    void* event_data)
{
    ESP_LOGD(TAG, "Handling a WIFI event");
    if (event_id == WIFI_EVENT_STA_START) {
        ESP_LOGD(TAG, "STA_START");
        esp_wifi_connect();
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGD(TAG, "DISCONNECTED");
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retrying to connect to the AP");
    } else if (event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGD(TAG, "GOT_IP");
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        char buffer[20];
        ESP_LOGI(TAG, "Conncted to wifi with IP address: %s", esp_ip4addr_ntoa(&event->ip_info.ip, buffer, 20));
    }
}

// Set up non-volatile storage
static esp_err_t setup_nvs() {
    esp_err_t err;
    int attemptsLeft = 3;
    esp_err_t ret;
    do {
        ESP_LOGI(TAG, "Attempting to set up NVS");
        ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_LOGE(TAG, "Error in nvs init: %s. Trying to flash erase", esp_err_to_name(ret));
            err = nvs_flash_erase();
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Failed to flash erase: %s", esp_err_to_name(err));
            }
        }
        attemptsLeft--;
    } while((ret != ESP_OK) && (attemptsLeft != 0));
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize NVS: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "NVS has initialized successfully");
    return ESP_OK;
}

// Set up wifi connection
esp_err_t setup_wifi_helper() {
    esp_err_t err;
    // Set up wifi
    wifi_init_config_t init_config = WIFI_INIT_CONFIG_DEFAULT();
    err = esp_wifi_init(&init_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize wifi stack: %s", esp_err_to_name(err));
        return err;
    }

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {  
                .capable = true,
                .required = false
            }
        }
    };
    err = esp_wifi_set_mode(WIFI_MODE_STA);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set wifi mode: %s", esp_err_to_name(err));
        return err;
    }
    err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set wifi config: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register event handler for ANY: %s", esp_err_to_name(err));
        return err;
    }
    err = esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register event handler for GOT IP: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_wifi_start();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start wifi system: %s", esp_err_to_name(err));
        return err;
    }
    ESP_LOGI(TAG, "Wifi system started");
    return ESP_OK;
}

void log_mac_address() {
    esp_err_t err;
    // Get the MAC address for the Wi-Fi station
    uint8_t mac_addr[6] = {0};
    err = esp_wifi_get_mac(ESP_IF_WIFI_STA, mac_addr);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get MAC address: %s", esp_err_to_name(err));
        return;
    }

    // Log the MAC address
    ESP_LOGI(TAG, "MAC Address: %02x:%02x:%02x:%02x:%02x:%02x",
        mac_addr[0], mac_addr[1], mac_addr[2],
        mac_addr[3], mac_addr[4], mac_addr[5]);
}

esp_err_t setup_wifi() {
    // Set up non-volatile storage
    ESP_ERROR_CHECK(setup_nvs());
    // Some other required initializations
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    // Connect to WIFI
    ESP_ERROR_CHECK(setup_wifi_helper());
    log_mac_address();
    return ESP_OK;
}
