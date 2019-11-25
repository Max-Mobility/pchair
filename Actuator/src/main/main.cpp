#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include "sdkconfig.h"


#define MS_TO_TICKS(xTimeInMs)                                                 \
  (uint32_t)(((TickType_t)xTimeInMs * configTICK_RATE_HZ) / (TickType_t)1000)

// include the task components
#include "../components/BLE/include/gatts.hpp"
#include "../components/Timer/include/Timer.hpp"
#include "Actuator.hpp"
#include "Display.hpp"
#include "DisplayTask.hpp"
#include "I2C.hpp"
#include "SerialTask.hpp"
#include "TFT.hpp"
#include "WirelessTask.hpp"
#include "motor.hpp"

// include the timer components

// now start the tasks that have been defined
extern "C" void app_main(void) {
  printf("hello world!");

  // clear_vram();

  //  // create the tasks
  //  xTaskCreate(&WirelessTask::taskFunction, // function the task runs
  //	      "taskFunction_0", // name of the task (should be short)
  //	      2048, // stack size for the task
  //	      NULL, // parameters to task
  //	      0, // priority of the task (higher -> higher priority)
  //	      NULL // returned task object (don't care about storing it)
  //	      );
  xTaskCreate(&Motor::taskFunction, // function the task runs
              "taskFunction_0",     // name of the task (should be short)
              2048,                 // stack size for the task
              NULL,                 // parameters to task
              0,   // priority of the task (higher -> higher priority)
              NULL // returned task object (don't care about storing it)
  );
  xTaskCreate(&SerialTask::taskFunction, // function the task runs
              "taskFunction_1",          // name of the task (should be short)
              4096,                      // stack size for the task
              NULL,                      // parameters to task
              0,   // priority of the task (higher -> higher priority)
              NULL // returned task object (don't care about storing it)
  );
  xTaskCreate(&DisplayTask::taskFunction, // function the task runs
              "taskFunction_2",           // name of the task (should be short)
              2048,                       // stack size for the task
              NULL,                       // parameters to task
              0,   // priority of the task (higher -> higher priority)
              NULL // returned task object (don't care about storing it)
  );
  xTaskCreate(&Actuator::taskFunction, // function the task runs
              "taskFunction_3",        // name of the task (should be short)
              4096,                    // stack size for the task
              NULL,                    // parameters to task
              0,   // priority of the task (higher -> higher priority)
              NULL // returned task object (don't care about storing it)
  );
  xTaskCreate(&BLE::BLEtaskFunction, // function the task runs
              "taskFunction_BLE",    // name of the task (should be short)
              2048,                  // stack size for the task
              NULL,                  // parameters to task
              0,   // priority of the task (higher -> higher priority)
              NULL // returned task object (don't care about storing it)
  );
  start_timer();

  //  TFT_init();
  I2C::start_I2C();
}
