#include "motor.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "I2C.hpp"


namespace Motor{
	uint8_t forwardSpeedLimit = 50;
	uint8_t rotationSpeedLimit = 30;
	void caculateMotorSpeed(uint8_t joyX,uint8_t joyY,Actuator::system_modes mode)
	{
		float x =0;
		float y =0;
		float left = 0;
		float right =0;
		float factor = 1;
		if (mode != Actuator::system_modes::DriveMode)
		{
//			printf("joystick error!\n");
			SerialTask::leftSpeed[0] = char(0);
			SerialTask::rightSpeed[0] = char(0);
		}
		else
		{
			x = converterJoystickReading(joyX);
			y = converterJoystickReading(joyY);
			left = (y*forwardSpeedLimit/100.0+x*rotationSpeedLimit/100.0);
			right =( y*forwardSpeedLimit/100.0 - x*rotationSpeedLimit/100.0);
//			printf("joyx=%d,joyY=%d,left=%f,right=%f  ",joyX,joyY,left,right);
			if (abs(left)>forwardSpeedLimit)
			{
				factor = forwardSpeedLimit/float(abs(left));
			}
			if (abs(right)>forwardSpeedLimit)
			{
				factor = forwardSpeedLimit/float(abs(right));
			}
			left = left*factor;
			right= -1*right*factor; //motor mount reversed.
			SerialTask::leftSpeed[0] = char(left);
			SerialTask::rightSpeed[0] = char(right);
			//printf("left=%f,right=%f\n",left,right);
		}

	}

	float converterJoystickReading(uint8_t r)
	{
		float x=0;
		if (r>joyStickZeroMax)
		{
			x =( (r -joyStickZeroMax)*100.0/(255-joyStickZeroMax));
		}
		else
		{
			if (r < joyStickZeroMin)
			{
				x =( (r-joyStickZeroMin)*100.0/joyStickZeroMin);
			}
			else
				x=0;
		}
		return x;

	}

	void taskFunction ( void *pvParameter ) {

		while(1)
		{


			caculateMotorSpeed(I2C::joystickX,I2C::joystickY,Actuator::systemMode);
			vTaskDelay((20 * (1)) / portTICK_RATE_MS);

		}


	}

}
