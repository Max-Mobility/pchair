#ifndef __Motor__INCLUDE_GUARD
#define __Motor__INCLUDE_GUARD

#include <cstdint>
#include <stdint.h>
#include <stdio.h>
// Task Includes
#include "Actuator.hpp"
#include "I2C.hpp"
#include "SerialTask.hpp"
#include "driver/gpio.h" // needed for printf
#include "gatts.hpp"


namespace Motor {
#define joyStickXZeroMin 120
#define joyStickXZeroMax 140
#define joyStickXMin 27
#define joyStickXMax 225
#define joyStickYZeroMin 114
#define joyStickYZeroMax 134
#define joyStickYMin 36
#define joyStickYMax 212

extern uint8_t forwardSpeedLimit;
extern uint8_t rotationSpeedLimit;
extern uint8_t phone_joystickX;
extern uint8_t phone_joystickY;
void caculateSpeedLimit(void);
void caculateMotorSpeed(float x, float y, Actuator::system_modes mode);
float converterJoystickReadingX(uint8_t r);
float converterJoystickReadingY(uint8_t r);
float converterJoystickReadingPhone(uint8_t r);
float caculateSpeedRamp(float currSpeed, float targetSpeed);

void taskFunction(void *pvParameter);

}; // namespace Motor

#endif // __Motor__INCLUDE_GUARD
