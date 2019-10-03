#ifndef __Motor__INCLUDE_GUARD
#define __Motor__INCLUDE_GUARD

#include <cstdint>
#include <stdint.h>
#include <stdio.h>
// Task Includes
#include "driver/gpio.h" // needed for printf
#include "I2C.hpp"
#include "SerialTask.hpp"
#include "Actuator.hpp"

namespace Motor{
	#define joyStickZeroMin 118
	#define joyStickZeroMax 137
	extern uint8_t forwardSpeedLimit;
	extern uint8_t rotationSpeedLimit;
	void caculateMotorSpeed(uint8_t joyX,uint8_t joyY,Actuator::system_modes mode);
	float converterJoystickReading(uint8_t r);

	void  taskFunction ( void *pvParameter );




};



#endif // __Motor__INCLUDE_GUARD
