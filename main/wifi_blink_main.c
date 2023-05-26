/**
 * @file wifi_blink_main.c
 * @author Michael Uman (muman613@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2023-05-24
 * 
 * @copyright Copyright (c) UmanSoft 2023
 * 
 */
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include <stdio.h>

#include "cJSON.h"
#include "mqtt_utils.h"
#include "wifi.h"

const char *TAG = "wifi station";

/* Use project configuration menu (idf.py menuconfig) to choose the GPIO to
   blink, or you can edit the following line and set a number here.
*/
#define BLINK_GPIO CONFIG_BLINK_GPIO

static uint8_t s_led_state = 0;


static void blink_led(void) {
  /* Set the GPIO level according to the state (LOW or HIGH)*/
  gpio_set_level(BLINK_GPIO, s_led_state);
}

static void configure_led(void) {
  ESP_LOGI(TAG, "Example configured to blink GPIO LED!");
  gpio_reset_pin(BLINK_GPIO);
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}


static void flash_led_timer_callback(TimerHandle_t xTimer) {
  ESP_LOGI(TAG, "Timer 1 callback");
  blink_led();
  publish_led_state(s_led_state, CONFIG_BLINK_PERIOD);
  /* Toggle the LED state */
  s_led_state = !s_led_state;
}

void app_main(void) {
  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  /* Configure the peripheral according to the LED type */
  configure_led();
  /* Connect to Wifi */
  wifi_init_sta();

  ESP_LOGI(TAG, "Connecting to MQTT");
  client_ = mqtt_app_start();

  /* Send connected message to MQTT broker */
  publish_connection_state(true);

  TimerHandle_t timer1 =
      xTimerCreate("toggler", pdMS_TO_TICKS(CONFIG_BLINK_PERIOD), pdTRUE, 0, flash_led_timer_callback);
  if (timer1 != NULL) {
    ESP_LOGI(TAG, "Starting timer");
    if (xTimerStart(timer1, 0) != pdPASS) {
      ESP_LOGE(TAG, "Failed to start");
    }
  }

}
