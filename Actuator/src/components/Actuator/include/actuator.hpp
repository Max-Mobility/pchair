
#ifndef __Actuator__INCLUDE_GUARD
#define __Actuator__INCLUDE_GUARD

#include <stdint.h>
#include "driver/gpio.h"

namespace Actuator {

	#define pwmPin1 GPIO_NUM_34
	#define pwmPin2 GPIO_NUM_35
	#define pwmPin3 GPIO_NUM_36
	#define pwmPin4 GPIO_NUM_27  // 34-39 are input only pins

	#define act1ControlA GPIO_NUM_18
	#define act1ControlB GPIO_NUM_19
	#define act2ControlA GPIO_NUM_21
	#define act2ControlB GPIO_NUM_22
	#define act3ControlA GPIO_NUM_14
	#define act3ControlB GPIO_NUM_26
	#define act4ControlA GPIO_NUM_12
	#define act4ControlB GPIO_NUM_25
	#define ActReclineOutLimit 100
	#define ActLegrestOutLimit 92//86
	#define ActTiltOutLimit 92
	#define ActElevationOutLimit 100
	#define GoKartHeight 60
	//#define actEnable GPIO_NUM_14

	enum system_modes
	{
		DriveMode,
		Actuator_recline,
		Actuator_legrest,
		Actuator_tilt,
		Actuator_elevation,
		Actuator_stand,
		System_sleep,
		ErrorMode,
		system_mode_max,
	};
	enum joystick_direction
	{
		middle,
		left,
		right,
		up,
		down,
	};
	enum action_input
	{
		nonMoving,
		movingUp,
		movingDown,
		movingLeft,
		movingRight,
	};
	enum Elevation_state
	{
		seat_state,
		GoKart_state,
		floor_state,
	};


	extern system_modes systemMode;
	extern system_modes last_system_modes;
	extern joystick_direction joyDir;
	extern bool system_mode_changed;

	extern float speedLimit;
	void taskFunction ( void *pvParameter );
	void controlPinSetup(gpio_num_t gpio_num);
	joystick_direction get_joyDir(uint8_t x, uint8_t y);
	void updateDirCounter();
	action_input getActionInput(action_input act);
	void DriveToTarget(int32_t recline, int32_t tilt, int32_t leg,int32_t elev );

	void actuator_init();
	void updatePosition();
	void updateCounter();
	void updateAngle();
	void actuatorStop();
	void checkButton();
	void checkReclineAngle();
	void checkLegRestHeight();
	void maintainSeatAngle();
	float getBackRestAngle(uint32_t pos);
	float getLegRestAngle(uint32_t pos);
	float getChassisAngle(uint32_t pos);
	float getSeatAngle(uint32_t pos);
	float getLegRestJointLength(uint32_t pos);
	float getChassisAngle(uint32_t pos);
	bool LegRestTouchGround();
	bool ReclineAngleInRange();
	bool LegRestInRange();
	void updateActPositionLimit();



	extern float SeatAngleToGround;
	extern float BackAngleToGround;
	extern float LegRestHeightToGround;
	extern float ChassisAngleToGround;
	extern float LegRestAngleToGround;
	uint32_t getPosFromAngle_BackRest(float angle);
	uint32_t getPosFromAngle_Chassis(float angle);
	uint32_t getPosFromAngle_LegRest(float angle);
	uint32_t getPosFromAngle_Seat(float angle);


	class ACT
	{
		public:
			gpio_num_t controlA;
			gpio_num_t controlB;
			gpio_num_t pwmInput;
			uint32_t counter_high;
			uint32_t counter_low;
			uint32_t position;
			uint32_t position_memory;
			bool positionUpdated;
			uint32_t pwm_up_limit;  //could be 100 or 90
			uint32_t pwm_low_limit; // could be 0 or 10
			uint32_t position_out_limit;
			uint32_t position_in_limit;
			uint32_t target;
			bool targetUpdated;
			ACT(gpio_num_t A, gpio_num_t B,gpio_num_t P,uint32_t up,uint32_t low,uint32_t out);
			void initPWM(gpio_num_t gpio_num);
			void updateCounter(void);
			void getPosition(void);
			void moveToTarget();
			void update();
			void moveIn();
			void moveOut();
			void stop();
			void setTarget(uint32_t T);
			void moveToMemory();


	};

	extern ACT actRecline;
	extern ACT actLegrest;
	extern ACT actTilt;
	extern ACT actElevation;
}




#endif
