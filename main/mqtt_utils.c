#include "mqtt_utils.h"
#include "cJSON.h"
#include "esp_log.h"
#include "sdkconfig.h"

esp_mqtt_client_handle_t client = NULL;
extern const char *TAG;

static char *getTopic(const char *topic) {
  static char topicBuff[NAME_MAX];
  snprintf(topicBuff, NAME_MAX, "/topic/%s", topic);
  return topicBuff;
}

static void log_error_if_nonzero(const char *message, int error_code) {
  if (error_code != 0) {
    ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
  }
}
/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                               int32_t event_id, void *event_data) {
  ESP_LOGD(TAG,
           "Event dispatched from event loop base=%s, event_id=%" PRIi32 "",
           base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:

    msg_id = esp_mqtt_client_subscribe(client, getTopic(CONFIG_DEVICE_ID), 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

    // msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
    // ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    break;

  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    // msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
    // ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
      log_error_if_nonzero("reported from esp-tls",
                           event->error_handle->esp_tls_last_esp_err);
      log_error_if_nonzero("reported from tls stack",
                           event->error_handle->esp_tls_stack_err);
      log_error_if_nonzero("captured as transport's socket errno",
                           event->error_handle->esp_transport_sock_errno);
      ESP_LOGI(TAG, "Last errno string (%s)",
               strerror(event->error_handle->esp_transport_sock_errno));
    }
    break;
  default:
    ESP_LOGI(TAG, "Other event id:%d", event->event_id);
    break;
  }
}

esp_mqtt_client_handle_t mqtt_app_start(void) {
  esp_mqtt_client_config_t mqtt_cfg = {
      .broker.address.uri = "mqtt://umansoft.ddns.net",
  };

  esp_mqtt_client_handle_t my_client = esp_mqtt_client_init(&mqtt_cfg);
  /* The last argument may be used to pass data to the event handler, in this
   * example mqtt_event_handler */
  esp_mqtt_client_register_event(my_client, ESP_EVENT_ANY_ID,
                                 mqtt_event_handler, NULL);
  esp_mqtt_client_start(my_client);

  return my_client;
}

const char *create_led_state_json(const char *state, int delay) {
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

  const char *state_json = cJSON_PrintUnformatted(json);

  cJSON_Delete(json);
  return state_json;
}

const char *create_connection_state_json(bool connected) {
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

  const char *state_json = cJSON_PrintUnformatted(json);

  cJSON_Delete(json);
  return state_json;
}

/**
 * @brief Send LED state to MQTT broker...
 *
 * @param ledState true/false indicating on/off status
 * @param delay ms delay value
 */
void publish_led_state(bool ledState, int delay) {
  const char *led_json_blob =
      create_led_state_json(ledState ? "On" : "Off", CONFIG_BLINK_PERIOD);
  esp_mqtt_client_publish(client, getTopic(CONFIG_DEVICE_ID), led_json_blob,
                          strlen(led_json_blob), 1, false);
}

/**
 * @brief Send LED state to MQTT broker...
 *
 * @param ledState true/false indicating on/off status
 * @param delay ms delay value
 */
void publish_connection_state(bool connected) {
  const char *connection_json_blob =
      create_connection_state_json(connected);
  esp_mqtt_client_publish(client, getTopic("connect"), connection_json_blob,
                          strlen(connection_json_blob), 1, false);
}
