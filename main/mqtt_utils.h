
#pragma once

#include "mqtt_client.h"

extern esp_mqtt_client_handle_t client_;


esp_mqtt_client_handle_t mqtt_app_start(void);
void publish_led_state(bool ledState, int delay);
void publish_connection_state(bool connected);

