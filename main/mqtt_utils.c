#include <stdbool.h>
#include "mqtt_utils.h"
#include "json.h"
#include "esp_log.h"
#include "sdkconfig.h"

esp_mqtt_client_handle_t client_ = NULL;
extern const char *TAG;



static char *getTopic(const char *topic) {
  static char topicBuff[NAME_MAX];
  snprintf(topicBuff, NAME_MAX, "/topic/%s/%s", CONFIG_DEVICE_ID, topic);
  return topicBuff;
}

static char *getSysTopic(const char *topic) {
  static char topicBuff[NAME_MAX];
  snprintf(topicBuff, NAME_MAX, "/topic/system/%s", topic);
  return topicBuff;
}

static void log_error_if_nonzero(const char *message, int error_code) {
  if (error_code != 0) {
    ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
  }
}

/**
 * @brief Handle MQTT subscription reception...
 * 
 * @param topic Topic of message received
 * @param msg Message data in null terminated string...
 */
static void on_topic_received(char *topic, char *msg) {
  ESP_LOGI(TAG, "TOPIC=%s", topic);
  ESP_LOGI(TAG, "DATA=%s", msg);
}

void subscribe_to_topics(esp_mqtt_client_handle_t client) {
  int msg_id;

  msg_id = esp_mqtt_client_subscribe(client, getTopic("control"), 0);
  ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
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
  // int msg_id;
  switch ((esp_mqtt_event_id_t)event_id) {
  case MQTT_EVENT_CONNECTED:
    subscribe_to_topics(client);
    // msg_id = esp_mqtt_client_subscribe(client, getTopic(CONFIG_DEVICE_ID),
    // 0); ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

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
    {
      // printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
      // printf("DATA=%.*s\r\n", event->data_len, event->data);
      char *topic = NULL, *data = NULL;
      topic = (char *)malloc(event->topic_len + 1);
      data = (char *)malloc(event->data_len + 1);
      memset(topic, 0, event->topic_len + 1);
      memset(data, 0, event->data_len + 1);
      strncpy(topic, event->topic, event->topic_len);
      strncpy(data, event->data, event->data_len);
      on_topic_received(topic, data);
      free(topic);
      free(data);
    }
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
  const char *lwt_msg = create_connection_state_json(false);

  esp_mqtt_client_config_t mqtt_cfg = {
      .broker.address.uri = "mqtt://umansoft.ddns.net",
      .session.keepalive = 30,
      .session.last_will.topic = getSysTopic("connection"),
      .session.last_will.msg = lwt_msg,
      .session.last_will.msg_len = strlen(lwt_msg),
      .session.last_will.retain = true,
      .session.last_will.qos = 1,
  };

  esp_mqtt_client_handle_t my_client = esp_mqtt_client_init(&mqtt_cfg);
  /* The last argument may be used to pass data to the event handler, in this
   * example mqtt_event_handler */
  esp_mqtt_client_register_event(my_client, ESP_EVENT_ANY_ID,
                                 mqtt_event_handler, NULL);
  esp_mqtt_client_start(my_client);

  free((void *)lwt_msg);
  return my_client;
}

/**
 * @brief Send LED state to MQTT broker...
 *
 * @param ledState true/false indicating on/off status
 * @param delay ms delay value
 */
void publish_led_state(bool ledState, int delay) {
  char *led_json_blob =
      create_led_state_json(ledState ? "On" : "Off", CONFIG_BLINK_PERIOD);
  esp_mqtt_client_publish(client_, getTopic("status"), led_json_blob,
                          strlen(led_json_blob), 1, false);
  free((void *)led_json_blob);
}

/**
 * @brief Send connection state to MQTT broker...
 *
 * @param connected true/false indicating connection status
 */
void publish_connection_state(bool connected) {
  char *connection_json_blob = create_connection_state_json(connected);
  esp_mqtt_client_publish(client_, getSysTopic("connection"),
                          connection_json_blob, strlen(connection_json_blob), 1,
                          true);
  free(connection_json_blob);
}
