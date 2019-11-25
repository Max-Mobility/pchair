/* esp_timer (high resolution timer) example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include "include/Timer.hpp"

#include "DisplayTask.hpp"
#include "I2C.hpp"
#include "actuator.hpp"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "sdkconfig.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void periodic_timer_callback_7500HZ(void *arg);
static void periodic_timer_callback_2HZ(void *arg);
std::string str;

void start_timer(void) {

  //	controlPinSetup(actEnable);
  /* Create two timers:
   * 1. a periodic timer which will run every 0.5s, and print a message
   * 2. a one-shot timer which will fire after 5s, and re-start periodic
   *    timer with period of 1s.
   */
  esp_timer_create_args_t periodic_timer_args_2HZ;
  periodic_timer_args_2HZ.callback = &periodic_timer_callback_2HZ;
  periodic_timer_args_2HZ.dispatch_method = ESP_TIMER_TASK;
  periodic_timer_args_2HZ.name =
      "periodic_2HZ"; /* name is optional, but may help identify the timer when
                         debugging */

  esp_timer_handle_t periodic_timer_2HZ;
  ESP_ERROR_CHECK(
      esp_timer_create(&periodic_timer_args_2HZ, &periodic_timer_2HZ));
  /* The timer has been created but is not running yet */

  esp_timer_create_args_t periodic_timer_args_7500HZ;
  periodic_timer_args_7500HZ.callback = &periodic_timer_callback_7500HZ;
  periodic_timer_args_7500HZ.dispatch_method = ESP_TIMER_TASK;
  periodic_timer_args_7500HZ.name =
      "periodic_7500HZ"; /* name is optional, but may help identify the timer
                            when debugging */

  esp_timer_handle_t periodic_timer_7500HZ;
  ESP_ERROR_CHECK(
      esp_timer_create(&periodic_timer_args_7500HZ, &periodic_timer_7500HZ));
  /* The timer has been created but is not running yet */

  /* Start the timers */
  ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer_2HZ, 2000000 / 25));
  ESP_ERROR_CHECK(
      esp_timer_start_periodic(periodic_timer_7500HZ, 1000000 / 7500));
  //    ESP_LOGI(TAG, "Started timers, time since boot: %lld us",
  //    esp_timer_get_time());
}

static void periodic_timer_callback_2HZ(void *arg) {
  // printf("this is the 2Hz timer\n");
  Actuator::updatePosition(); // calculate position by compare high and low
                              // counter.

  //	act1.update();
  //	act2.update();
  //	act3.update();
  //	act4.update();
  //	Actuator::DriveActManually();
  //	str.clear();
  //	str = "Mode:"+std::to_string(I2C::controlMode);
  //	str =str +". pwm:";
  //	str =str +std::to_string(Actuator::act1.position);
  //	str =str + ". Target:";
  //	str =str +std::to_string(Actuator::act1.target);
  //	str =str +"\n";
  //	DisplayTask::pushData(str);
}
static void periodic_timer_callback_7500HZ(void *arg) {
  Actuator::updateCounter(); // update  pwm high and low counter
}
