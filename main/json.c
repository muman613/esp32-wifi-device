#include "json.h"
#include "sdkconfig.h"
#include <stdbool.h>

char *create_led_state_json(const char *state, int delay) {
  cJSON *json = NULL;
  json = cJSON_CreateObject();
  cJSON *control = cJSON_AddObjectToObject(json, "control");
  if (cJSON_AddStringToObject(control, "led", state) == NULL) {
    cJSON_Delete(json);
    return NULL;
  }
  if (cJSON_AddNumberToObject(control, "delay", delay) == NULL) {
    cJSON_Delete(json);
    return NULL;
  }

  char *state_json = cJSON_PrintUnformatted(json);

  cJSON_Delete(json);
  return state_json;
}

char *create_connection_state_json(bool connected) {
  cJSON *json = NULL;
  json = cJSON_CreateObject();
  cJSON *control = cJSON_AddObjectToObject(json, "connectivity");
  if (cJSON_AddStringToObject(control, "deviceId", CONFIG_DEVICE_ID) == NULL) {
    cJSON_Delete(json);
    return NULL;
  }
  if (cJSON_AddBoolToObject(control, "connected", connected) == NULL) {
    cJSON_Delete(json);
    return NULL;
  }

  char *state_json = cJSON_PrintUnformatted(json);

  cJSON_Delete(json);
  return state_json;
}