#include "motor.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "I2C.hpp"



namespace Motor{
	uint8_t forwardSpeedLimit = 70;
	uint8_t rotationSpeedLimit = 17;
	uint8_t phone_joystickX=128;
	uint8_t phone_joystickY=128;
	float joyXReading = 0;
	float joyYReading = 0;
	void caculateSpeedLimit(void)
	{	float speedSet = 0.5;
		switch(BLE::speedSettings)
		{
			case BLE::Speed_setting::speed_low:
				speedSet = 0.5;
				rotationSpeedLimit = 10;
			break;
			case BLE::Speed_setting::speed_medium:
				speedSet =0.7;
				rotationSpeedLimit = 17;
			break;
			case BLE::Speed_setting::speed_high:
				speedSet =1.0;
				rotationSpeedLimit = 25;
			break;
		}
		forwardSpeedLimit = uint8_t( speedSet*Actuator::speedLimit*100);
		//rotationSpeedLimit = uint8_t(speedSet*Actuator::speedLimit*30);

	}
	void caculateMotorSpeed(float x,float y,Actuator::system_modes mode)
	{

		float left = 0;
		float right =0;
		float factor = 1;
		if ((mode != Actuator::system_modes::DriveMode )&&(mode != Actuator::system_modes::PhoneControlMode))
		{
//			printf("joystick error!\n");
			SerialTask::leftSpeed[1] = char(0);
			SerialTask::rightSpeed[1] = char(0);
		}
		else
		{
			caculateSpeedLimit();

			left = (y*forwardSpeedLimit/100.0+x*rotationSpeedLimit/100.0);
			right =( y*forwardSpeedLimit/100.0 - x*rotationSpeedLimit/100.0);
//			printf("joyx=%f,joyY=%f,left=%f,right=%f \n ",x,y,left,right);
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
			SerialTask::leftSpeed[1] = char(left);
			SerialTask::rightSpeed[1] = char(right);
//			printf("left=%f,right=%f,joyx=%d,joyY=%d\n",left,right,joyX,joyY);
		}

	}

	float converterJoystickReadingX(uint8_t r)
	{
		float x=0;
		if (r>joyStickXMax)
		{
			r = joyStickXMax;
		}
		if (r<joyStickXMin)
		{
			r=joyStickXMin;
		}
		if (r>joyStickXZeroMax)
		{
			x =( (r -joyStickXZeroMax)*100.0/(joyStickXMax-joyStickXZeroMax));
		}
		else
		{
			if (r < joyStickXZeroMin)
			{
				x =( (r-joyStickXZeroMin)*100.0/(joyStickXZeroMin-joyStickXMin));
			}
			else
				x=0;
		}
		return x;

	}

	float converterJoystickReadingY(uint8_t r)
	{
		float x=0;
		if (r>joyStickYMax)
		{
			r = joyStickYMax;
		}
		if (r<joyStickYMin)
		{
			r=joyStickYMin;
		}
		if (r>joyStickYZeroMax)
		{
			x =( (r -joyStickYZeroMax)*100.0/(joyStickYMax-joyStickYZeroMax));
		}
		else
		{
			if (r < joyStickYZeroMin)
			{
				x =( (r-joyStickYZeroMin)*100.0/(joyStickYZeroMin-joyStickYMin));
			}
			else
				x=0;
		}
		return x;
	}

	float converterJoystickReadingPhone(uint8_t r)
		{
			float x=0;
			if (r>138)
			{
				x =( (r -138)*100.0/(255-138));
			}
			else
			{
				if (r < 118)
				{
					x =( (r-118)*100.0/118);
				}
				else
					x=0;
			}
			return x;

		}


	void taskFunction ( void *pvParameter ) {

		while(1)
		{
			if ( Actuator::systemMode==Actuator::system_modes::PhoneControlMode )
			{
				joyXReading = converterJoystickReadingPhone(phone_joystickX);
				joyYReading = converterJoystickReadingPhone(phone_joystickY);

			}
			else
			{
				joyXReading = converterJoystickReadingX(I2C::joystickX);
				joyYReading = converterJoystickReadingY(I2C::joystickY);

			}
			caculateMotorSpeed(joyXReading,joyYReading,Actuator::systemMode);
			vTaskDelay((40 * (1)) / portTICK_RATE_MS);

		}


	}

}
