#ifndef __SerialTask__INCLUDE_GUARD
#define __SerialTask__INCLUDE_GUARD

#include <cstdint>

// Task Includes
#include "DisplayTask.hpp"
#include "driver/gpio.h" // needed for printf
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "soc/uart_struct.h"
#include <string.h>
#include <string>


// Generated state functions and members for the task
namespace SerialTask {

// Task Forward Declarations
extern bool changeState;
// current speed = speed[0], target speed= speed[1]
extern char leftSpeed[2];
extern char rightSpeed[2];

// Generated task function
void taskFunction(void *pvParameter);

// Generated state functions
void state_State_1_execute(void);
void state_State_1_setState(void);
void state_State_1_transition(void);
void state_State_1_finalization(void);

}; // namespace SerialTask

#endif // __SerialTask__INCLUDE_GUARD
