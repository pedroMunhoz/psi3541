#ifndef CONNECT_WIFI_H_
#define CONNECT_WIFI_H_

#include <string.h>
#include <stdio.h>

#define ESP_WIFI_SSID CONFIG_ESP_WIFI_SSID
#define ESP_WIFI_PASSWORD CONFIG_ESP_WIFI_PASSWORD
#define ESP_MAXIMUM_RETRY CONFIG_ESP_MAXIMUM_RETRY

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

extern int wifi_connect_status;

void connect_wifi(void);

#endif