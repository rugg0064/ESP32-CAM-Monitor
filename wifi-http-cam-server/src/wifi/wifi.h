#ifndef WIFI_H_   /* Include guard */
#define WIFI_H_

#include "esp_err.h"

#define WIFI_SSID "PUT_YOUR_SSID"
#define WIFI_PASS "PUT_YOUR_PASS"

esp_err_t setup_wifi();

#endif // WIFI_H_