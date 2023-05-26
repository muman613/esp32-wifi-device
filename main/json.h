#pragma once

#include "cJSON.h"
#include <stdbool.h>

/**
 * @brief Create a connection state json object
 * 
 * @param connected true for connected state, false for disconnected
 * @return char* Json blob null-terminated string
 */
char *create_connection_state_json(bool connected);
/**
 * @brief Create a led state json object
 * 
 * @param state "On/Off" string
 * @param delay LED flash delay in ms
 * @return char* Json blob null-terminated string
 */
char *create_led_state_json(const char *state, int delay);
