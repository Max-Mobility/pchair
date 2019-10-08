#ifndef __SerialTask__INCLUDE_GUARD
#define __SerialTask__INCLUDE_GUARD

#include <cstdint>

// Task Includes
#include "driver/gpio.h" // needed for printf
#include "driver/uart.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "soc/uart_struct.h"
#include <string.h>
#include <string>
#include "DisplayTask.hpp"

// Generated state functions and members for the task
namespace SerialTask {

  // Task Forward Declarations
  extern bool changeState;
  extern char leftSpeed[1];
  extern char rightSpeed[1];

  // Generated task function
  void  taskFunction ( void *pvParameter );

  // Generated state functions
  void  state_State_1_execute      ( void );
  void  state_State_1_setState     ( void );
  void  state_State_1_transition   ( void );
  void  state_State_1_finalization ( void );

};

#endif // __SerialTask__INCLUDE_GUARD