

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "sdkconfig.h"
#include "actuator.hpp"
#include "I2C.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "DisplayTask.hpp"
#include "Display.hpp"
#include <cmath>
#include "gatts.hpp"
#define MS_TO_TICKS( xTimeInMs ) (uint32_t)( ( ( TickType_t ) xTimeInMs * configTICK_RATE_HZ ) / ( TickType_t ) 1000 )
namespace Actuator{

	ACT actRecline(act1ControlA,act1ControlB,pwmPin1,90,10,ActReclineOutLimit);
	ACT actLegrest(act2ControlA,act2ControlB,pwmPin2,100,0,ActLegrestOutLimit);
	ACT actTilt(act3ControlA,act3ControlB,pwmPin3,90,10,ActTiltOutLimit);
	ACT actElevation(act4ControlA,act4ControlB,pwmPin4,90,10,ActElevationOutLimit);

	float speedLimit = 0.5;

	float SeatAngleToGround = 0;
	float BackAngleToGround =0;
	float LegRestHeightToGround=0;
	float ChassisAngleToGround =0;
	float LegRestAngleToGround =0;
	float LegRestJointAngleToGround=0;
	float LegRestJointHeight=0;
	float SeatHeight = 0;
	float BackAngleToSeat =0;
	float SeatAngleToChassis=0;
	float LegRestAngleToSeat = 0;
	float SavedSeatAngleToGround=0;

	int dirCounter_mid;
	int dirCounter_up;
	int dirCounter_down;
	int dirCounter_left;
	int dirCounter_right;
	int buttonCounter_up=0;
	int buttonCounter_low=0;

	bool stateChanged = false;     //when this is true, save actuator position in tilt or stand mode.
	bool system_mode_changed = true; //when this is true, send new system mode to the App.
	std::string str;

	enum Elevation_state elevationState = seat_state;

	action_input actionInput;
	system_modes systemMode = DriveMode;
	system_modes last_system_modes= DriveMode;
	joystick_direction joyDir=middle;
	int joyX=0;
	int joyY=0;
	uint32_t __state_delay__ = 100;
	uint32_t logCounter = 0;
	void  taskFunction ( void *pvParameter )
	{
		__state_delay__=100;
		actuator_init();
		dirCounter_mid =0;
		dirCounter_up=0;
		dirCounter_down=0;
		dirCounter_left=0;
		dirCounter_right=0;
		buttonCounter_up=0;
		buttonCounter_low=0;
		actionInput = nonMoving;

		std::string str;
		str = "   Driving";
		DisplayTask::pushData(str);
		str = "   Recline";
		DisplayTask::pushData(str);
		str = "   Legrest";
		DisplayTask::pushData(str);
		str = "   Tilt";
		DisplayTask::pushData(str);
		str = "   Elevation";
		DisplayTask::pushData(str);
		str = "   Stand";
		DisplayTask::pushData(str);
		draw_circle_inline(1);

		while(1)
		{
			if (I2C::joystickError)
			{
				systemMode = ErrorMode;
				actuatorStop();
				//printf("systemMode: %d\n",systemMode);
				if (last_system_modes!=systemMode )
				{
					stateChanged =true;
					system_mode_changed = true;
				}
				else
					stateChanged=false;
				last_system_modes=systemMode;
			}
			else
			{	actionInput = getActionInput(actionInput);
				updateAngle();
				checkButton();
				logCounter++;
//				if(logCounter%5==0) //1 log per second
//				{
//					printf("reclinePos:%d; LegPos:%d; TiltPos:%d; ElevPos:%d;   ",actRecline.position,actLegrest.position,actTilt.position,actElevation.position);
//				}
//				else
//				{
//					if(logCounter%5==0) //1 log per second
//					{
//						printf("Seat:%f; Back:%f; "
//								"Leg:%f; Chassis:%f; SavedSTC:%f; "
//								"LegH:%f mm;\n ",	SeatAngleToGround,BackAngleToGround,LegRestAngleToGround,
//								ChassisAngleToGround,SavedSeatAngleToGround,LegRestHeightToGround);
//					}
//				}
				if (last_system_modes!=systemMode )
				{
					system_mode_changed = true;
					stateChanged =true;
				}
				else
					stateChanged=false;
				last_system_modes=systemMode;
				switch (systemMode)
				{
					case System_sleep:
						break;
					case DriveMode:
						actuatorStop();
						if (Actuator::actElevation.position<GoKartHeight-3 )
						{
							speedLimit = 0.7;
						}
						else
						{
							if (Actuator::actElevation.position>GoKartHeight+3 )

								{
									speedLimit = 0.3;
								}
							else
							{
								speedLimit = 1.0;
							}
						}
						if (SeatAngleToGround<-10)
						{
							speedLimit = 0;
						}



						break;
					case Actuator_recline:                     // only move recline actuator
						updateActPositionLimit();
						switch (actionInput)
						{
							case nonMoving:
								actuatorStop();
								actRecline.stop();
								break;
							case movingUp:
								systemMode = system_modes::Actuator_stand;
								draw_circle_inline(6);
								break;
							case movingDown:
								systemMode =system_modes::Actuator_legrest;
								draw_circle_inline(3);
								break;
							case movingLeft:
								//actRecline.moveIn();
								if ((BackAngleToGround>85))
									actRecline.moveIn();
								else
									actRecline.stop();
								break;
							case movingRight:
								//actRecline.moveOut();
								if ((BackAngleToGround<175))
									actRecline.moveOut();
								else
									actRecline.stop();
								break;
						}
						break;
					case Actuator_legrest:         // only move legrest when legrest high are higher than 50mm
						switch (actionInput)
						{
							case nonMoving:
								actuatorStop();
								actLegrest.stop();
								break;
							case movingUp:
								systemMode = system_modes::Actuator_recline;
								draw_circle_inline(2);
								break;
							case movingDown:
								systemMode =system_modes::Actuator_tilt;
								draw_circle_inline(4);
								break;
							case movingLeft:
								//actLegrest.moveIn();
								if (LegRestHeightToGround>75)
									actLegrest.moveIn();
								else
									actLegrest.stop();
								break;
							case movingRight:
								//actLegrest.moveOut();
								if (LegRestHeightToGround>75)
									actLegrest.moveOut();
								else
									actLegrest.stop();
								break;
						}
						break;
					case Actuator_tilt:        //change seat angle by moving tilt actuator or elevation actuator or both
						updateActPositionLimit();
						switch (actionInput)
						{
							case nonMoving:
								actuatorStop();
								actTilt.stop();

								break;
							case movingUp:
								systemMode = system_modes::Actuator_legrest;
								draw_circle_inline(3);
								break;
							case movingDown:
								systemMode =system_modes::Actuator_elevation;

								draw_circle_inline(5);
								break;
							case movingLeft:
								//actTilt.moveIn();
								if (SeatAngleToChassis>6)
									actTilt.moveIn();
								else {
									actTilt.stop();
									actElevation.moveIn();
								}
								break;
							case movingRight:
								//actTilt.moveOut();
								if (SeatAngleToGround>0)
									actTilt.moveOut();
								else
									actTilt.stop();
								break;
						}
						break;
					case Actuator_elevation:   // move elevation actuator and tilt actuator to mantain same seat angle
						if (stateChanged)
						{
							SavedSeatAngleToGround = SeatAngleToGround;
//							printf("changed to elevation state.\n");
							if (Actuator::actElevation.position<GoKartHeight-3 )
							{
								elevationState = seat_state;
								speedLimit = 0.7;
							}
							else
							{
								if (Actuator::actElevation.position>GoKartHeight+3 )

									{
										elevationState = floor_state;
										speedLimit = 0.3;
									}
								else
								{
									elevationState = GoKart_state;
									speedLimit = 1.0;
								}
							}
						}
						switch (actionInput)
						{
							case nonMoving:
								actuatorStop();
								//actElevation.stop();
//								maintainSeatAngle();
								if (Actuator::actElevation.position<GoKartHeight-3 )
								{
									elevationState = seat_state;
								}
								else
								{
									if (Actuator::actElevation.position>GoKartHeight+3 )
										elevationState = floor_state;
									else
										elevationState = GoKart_state;
								}
								break;
							case movingUp:
								systemMode = system_modes::Actuator_tilt;
								draw_circle_inline(4);
								break;
							case movingDown:
								systemMode =system_modes::Actuator_stand;
								draw_circle_inline(6);
								break;
							case movingLeft:       // lower the elevation == moveOut
								switch(elevationState)
								{
									case seat_state:
										if (actElevation.position >GoKartHeight)
											actElevation.stop();
										else
											actElevation.moveOut();
										break;
									case GoKart_state:
										actElevation.moveOut();
										break;
									case floor_state:
										actElevation.moveOut();
										break;

								}
								maintainSeatAngle();
								checkLegRestHeight();
								checkReclineAngle();
								break;
							case movingRight:     // increase elevation by moving in
								if (LegRestTouchGround())
								{
									actElevation.stop();
								}
								else
								{
									switch(elevationState)
									{
										case seat_state:
											actElevation.moveIn();   // move to higher elevation
											break;
										case GoKart_state:
											actElevation.moveIn();   // move to higher elevation
											break;
										case floor_state:	// move to higher elevation until reach GoKartHeight
											if (actElevation.position>GoKartHeight)
											{
												actElevation.moveIn();
											}
											else
											{
												actElevation.stop();
											}
											break;
									}

								}
								maintainSeatAngle();
								checkLegRestHeight();
								checkReclineAngle();
								break;
						}
						break;
					case Actuator_stand:
						if (stateChanged)
						{

							actRecline.position_memory=actRecline.position;
							actLegrest.position_memory=actLegrest.position;
							actTilt.position_memory=actTilt.position;
							actElevation.position_memory=actElevation.position;
							SavedSeatAngleToGround = SeatAngleToGround;
							printf("Recline:%d; Leg:%d; Tilt:%d; Elev:%d;\n",actRecline.position_memory,actLegrest.position_memory,
									actTilt.position_memory,actElevation.position_memory);

						}
						switch (actionInput)
						{
							case nonMoving:
								actuatorStop();
								break;
							case movingUp:
								systemMode = system_modes::Actuator_elevation;
								draw_circle_inline(5);
								break;
							case movingDown:
								systemMode =system_modes::Actuator_recline;
								draw_circle_inline(2);
								break;
							case movingLeft:         //move from standing to saved position
								if (LegRestInRange())
								{
									actElevation.moveToMemory();
									actLegrest.moveToMemory();
								}
								else
								{
									actElevation.stop();
									actLegrest.stop();
								}
								if (ReclineAngleInRange())
								{
									actRecline.moveToMemory();
//									if (logCounter%5 ==0)
//									{
//										printf("Move Recline back!\n");
//									}
								}
								else
								{
									actRecline.stop();
								}
								actTilt.moveToMemory();


								break;
							case movingRight:   //move to standing position
								if (actElevation.position>0)
								{
									actElevation.moveIn();
								}
								else
								{
									actElevation.stop();
								}
								if(LegRestInRange())
								{
									if (actTilt.position<100)
									{
										actTilt.moveOut();
									}
									else
									{
										actTilt.stop();
									}
								}
								checkLegRestHeight();
								checkReclineAngle();

								break;
						}
						break;
					case ErrorMode:
						systemMode = system_modes::System_sleep;
						draw_circle_inline(1);
						break;
					default:
						break;
				}


			}
			vTaskDelay( MS_TO_TICKS(__state_delay__) );
		}

	}

	void checkButton()
	{
		if (I2C::joystickButtonHold_up && !I2C::joystickButtonHold_low)
		{
			buttonCounter_up++;
		}
		else
		{
			buttonCounter_up=0;
		}
		if (!I2C::joystickButtonHold_up && I2C::joystickButtonHold_low)
		{
			buttonCounter_low++;
		}
		else
		{
			buttonCounter_low=0;
		}
		if (buttonCounter_up==2)   // up button pressed to change mode from drive to act or from act to drive mode
		{
			if (systemMode==DriveMode)
			{
				systemMode = Actuator_recline;
			}
			else
			{
				if (systemMode!=DriveMode&& systemMode!=ErrorMode &&  systemMode!= System_sleep)
				{
					systemMode=DriveMode;
				}
			}

		}
		if (buttonCounter_low==2)  // go to sleep mode or wake up from sleep mode.
		{
			if (systemMode==System_sleep)
			{
				systemMode = DriveMode;
			}
			else
			{
				if (systemMode!=System_sleep&& systemMode!=ErrorMode)
				{
					systemMode=System_sleep;
				}
			}
		}
//		printf("system mode  %d, low_counter %d, up_counter %d\n",systemMode,buttonCounter_low,buttonCounter_up);
	}

	void actuatorStop()
	{
		actRecline.stop();
		actLegrest.stop();
		actTilt.stop();
		actElevation.stop();
	}
	action_input getActionInput(action_input act)
	{
		updateDirCounter();
		action_input _actionInput = nonMoving;
		if ((dirCounter_up%10==2)&& (BLE::actMovDir == BLE::Actuator_moving_dir::AMD_stop))
			_actionInput = movingUp;
		if ((dirCounter_down%10==2)&& (BLE::actMovDir == BLE::Actuator_moving_dir::AMD_stop))
			_actionInput = movingDown;
		if ((dirCounter_left>=2) ||(BLE::actMovDir == BLE::Actuator_moving_dir::AMD_left))
			_actionInput = movingLeft;
		if ((dirCounter_right>=2)||(BLE::actMovDir == BLE::Actuator_moving_dir::AMD_right))
			_actionInput = movingRight;
		if ((dirCounter_mid>=2) && (BLE::actMovDir == BLE::Actuator_moving_dir::AMD_stop))
			_actionInput = nonMoving;
		return _actionInput;
	}

	void updateDirCounter()
	{
		joyDir = get_joyDir(I2C::joystickX,I2C::joystickY);
		switch (joyDir)
		{
			case joystick_direction::middle:
				dirCounter_mid ++;
				dirCounter_up=0;
				dirCounter_down=0;
				dirCounter_left=0;
				dirCounter_right=0;
				break;
			case joystick_direction::up:
				dirCounter_mid =0;
				dirCounter_up++;
				dirCounter_down=0;
				dirCounter_left=0;
				dirCounter_right=0;
				break;
			case joystick_direction::down:
				dirCounter_mid =0;
				dirCounter_up=0;
				dirCounter_down++;
				dirCounter_left=0;
				dirCounter_right=0;
				break;
			case joystick_direction::left:
				dirCounter_mid =0;
				dirCounter_up=0;
				dirCounter_down=0;
				dirCounter_left++;
				dirCounter_right=0;
				break;
			case joystick_direction::right:
				dirCounter_mid =0;
				dirCounter_up=0;
				dirCounter_down=0;
				dirCounter_left=0;
				dirCounter_right++;
				break;
		}

	}

	joystick_direction get_joyDir(uint8_t x, uint8_t y)
	{
		int joyX=0;
		int joyY=0;
		joyX = x-127;
		joyY = y -127;
		joystick_direction _dir = middle;

		if ((joyX>60 ) &&(joyX>joyY)&&(joyX>-joyY)) // right
		{
			_dir = right;
		}
		else
		{
			if ((joyX<-60 ) &&(joyX<joyY)&&(joyX<-joyY)) // left
			{
				_dir=left;
			}

		}
		if ((joyY>60 ) &&(joyY>joyX)&&(joyY>-joyX)) // up
		{
			_dir=up;
		}
		else
		{
			if ((joyY<-60 ) &&(joyY<joyX)&&(joyY<-joyX)) // up
			{
				_dir=down;
			}

		}
		return _dir;


	}


	void actuator_init()
	{
		controlPinSetup(act1ControlA);
		controlPinSetup(act1ControlB);
		controlPinSetup(act2ControlA);
		controlPinSetup(act2ControlB);
		controlPinSetup(act3ControlA);
		controlPinSetup(act3ControlB);
		controlPinSetup(act4ControlA);
		controlPinSetup(act4ControlB);
		actuatorStop();
	}

	void updatePosition()
	{
		actRecline.getPosition();
		actLegrest.getPosition();
		actTilt.getPosition();
		actElevation.getPosition();
	}
	void updateCounter()
	{
		actRecline.updateCounter();
		actLegrest.updateCounter();
		actTilt.updateCounter();
		actElevation.updateCounter();

	}
	void ACT::initPWM(gpio_num_t gpio_num)
	{

		gpio_config_t io_conf;
		//disable interrupt
		io_conf.intr_type = GPIO_INTR_DISABLE;
		//set as output mode
		io_conf.mode = GPIO_MODE_INPUT;
		//bit mask of the pins that you want to set,e.g.GPIO18/19
		io_conf.pin_bit_mask = 1ULL<<gpio_num;
		//disable pull-down mode
		io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
		//disable pull-up mode
		io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
		//configure GPIO with the given settings
		gpio_config(&io_conf);


	}

	void controlPinSetup(gpio_num_t gpio_num)
	{
		gpio_config_t io_conf;
		//disable interrupt
		io_conf.intr_type = GPIO_INTR_DISABLE;
		//set as output mode
		io_conf.mode = GPIO_MODE_OUTPUT;
		//bit mask of the pins that you want to set,e.g.GPIO18/19
		io_conf.pin_bit_mask = 1ULL<<gpio_num;
		//disable pull-down mode
		io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
		//disable pull-up mode
		io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
		//configure GPIO with the given settings
		gpio_config(&io_conf);
	}

	void ACT::updateCounter()
	{
		if(gpio_get_level(ACT::pwmInput))
		{
			ACT::counter_high++;
		}
		else
		{
			ACT::counter_low++;
		}
	}

	void ACT::getPosition()
	{
		float pos =0.0;
		pos = (ACT::counter_high*100.0)/(ACT::counter_high+ACT::counter_low);
		pos = (pos-ACT::pwm_low_limit)*100.0/(ACT::pwm_up_limit-ACT::pwm_low_limit);
		if (pos>100)
			pos=100;
		if(pos<0)
			pos=0;
		ACT::position=uint32_t(pos);
		ACT::counter_high=0;
		ACT::counter_low=0;
		ACT::positionUpdated = true;
	}

	void ACT::moveToMemory()
	{
		int diff =ACT::position-ACT::position_memory;
		if (diff>3)
		{
			ACT::moveIn();
		}
		else
		{
			if(diff<-3)
			{
				ACT::moveOut();
			}
			else
			{
				ACT::stop();
			}
		}

	}
	ACT::ACT(gpio_num_t A, gpio_num_t B,gpio_num_t P,uint32_t up,uint32_t low,uint32_t out)
	{
		ACT::controlA = A;
		ACT::controlB = B;
		ACT::pwmInput = P;
		ACT::pwm_up_limit = up;
		ACT::pwm_low_limit = low;
		ACT::position_out_limit =out;    // 0-100% no limit
		ACT::position_in_limit = 0;      //initial to 0
		initPWM(ACT::pwmInput);
		controlPinSetup(A);
		controlPinSetup(B);
		ACT::targetUpdated = false;
		stop();
		ACT::counter_high=0;
		ACT::counter_low=0;
		ACT::positionUpdated = false;

	}


	void ACT::moveIn()
	{
		if (ACT::position>=ACT::position_in_limit)
		{
			gpio_set_level(controlB,1);
			gpio_set_level(controlA,1);
		}
		else
		{
			gpio_set_level(controlB,0);
		}

	}

	void ACT::moveOut()
	{


		if (ACT::position<=ACT::position_out_limit)
		{
			gpio_set_level(controlB,1);
			gpio_set_level(controlA,0);
		}
		else
		{
			gpio_set_level(controlB,0);
		}
	}

	void ACT::stop()
	{
		//gpio_set_level(controlA,0);
		gpio_set_level(controlB,0);
	}
	void ACT::moveToTarget()
	{
		if (ACT::target>ACT::position+2)
		{
			ACT::moveOut();
		}
		else
		{
			if (ACT::target<ACT::position-2)
				ACT::moveIn();
			else
				ACT::stop();

		}

	}
	void ACT::update()
	{
		if (ACT::positionUpdated)
		{
			if (!ACT::targetUpdated)  // first time get pwm position and set it as target.
			{
				ACT::target = ACT::position;
				ACT::targetUpdated=true;
			}
			else                     // compare with target and move the  actuator
			{
				if (ACT::target>ACT::position+2)
				{
					ACT::moveOut();
				}
				else
				{
					if (ACT::target<ACT::position-2)
						ACT::moveIn();
					else
						ACT::stop();
				}
			}
		}
	}

	void DriveToTarget(int32_t recline, int32_t tilt, int32_t leg,int32_t elev )  //target valid for 0-100, -1 for no action
	{
		if (recline>=0 )
		{
			actRecline.target=recline;
			actRecline.update();
		}
		if (tilt>=0)
		{
			actTilt.target=tilt;
			actTilt.update();
		}
		if(leg>=0)
		{
			actLegrest.target=leg;
			actLegrest.update();
		}
		if(elev>=0)
		{
			actElevation.target=elev;
			actElevation.update();
		}
	}
	float getBackRestAngle(uint32_t pos)
	{
		float angle=0;
		float p =(float)pos;
		p = p<0? 0:p;
		p = p>100? 100:p;
		angle = 0.0001225*p*p*p-0.0138339*p*p+0.8983*p+86.653;
		return angle;
	}
	float getLegRestAngle(uint32_t pos)
	{
		float angle=0;
		float p =(float)pos;
		p = p<0? 0:p;
		p = p>100? 100:p;
		angle = 0.0001814*p*p*p-0.0274*p*p+1.7569*p+85.176;
		return angle;
	}
	float getLegRestJointAngle(uint32_t pos)
	{
		float angle=0;
		float p =(float)pos;
		p = p<0? 0:p;
		p = p>100? 100:p;
		angle = 0.000165*p*p*p-0.02957*p*p+2.0738*p+90.7912;
		return angle;
	}
	float getLegRestJointLength(uint32_t pos)
	{
		float length=0;
		float p =(float)pos;
		p = p<0? 0:p;
		p = p>100? 100:p;
		length = -0.0000025*p*p*p+0.0056*p*p+0.8442*p+281.5455;
		return length;
	}
	float getChassisAngle(uint32_t pos)
	{
		float angle=0;
		float p =(float)pos;
		p = p<0? 0:p;
		p = p>100? 100:p;
		angle = -0.0000123*p*p*p+0.00278*p*p-0.6075*p+48.363;
		return angle;
	}
	float getSeatAngle(uint32_t pos)
	{
		float angle=0;
		float p =(float)pos;
		p = p<0? 0:p;
		p = p>100? 100:p;
		angle = 0.0000818*p*p*p-0.0116*p*p+1.5001*p+5.0561;
		return angle;
	}
	void updateAngle()
	{
		BackAngleToSeat =getBackRestAngle(Actuator::actRecline.position);
		SeatAngleToChassis=getSeatAngle(Actuator::actTilt.position);
		LegRestAngleToSeat = getLegRestAngle(Actuator::actLegrest.position);
		ChassisAngleToGround=getChassisAngle(Actuator::actElevation.position);

		SeatAngleToGround = ChassisAngleToGround -SeatAngleToChassis;
		BackAngleToGround = BackAngleToSeat+SeatAngleToGround;
		LegRestAngleToGround = LegRestAngleToSeat+SeatAngleToGround;
		LegRestJointAngleToGround = SeatAngleToGround+getLegRestJointAngle(Actuator::actLegrest.position);
		LegRestJointHeight = getLegRestJointLength(Actuator::actLegrest.position);
		SeatHeight = 90 + sin((ChassisAngleToGround-5)*3.14159/180.0)*550;
		LegRestHeightToGround = SeatHeight-sin((LegRestJointAngleToGround)*3.14159/180.0)*LegRestJointHeight;
	}
	void checkLegRestHeight()
	{
		if((LegRestHeightToGround<40)||(LegRestAngleToGround<90))//mm
		{
			actLegrest.moveOut();
		}
		else
		{
			actLegrest.stop();
		}
	}
	void checkReclineAngle()
	{
		if (BackAngleToGround>175)
			actRecline.moveIn();
		else
			if (BackAngleToGround<85)
				actRecline.moveOut();
			else
				actRecline.stop();

	}
	void maintainSeatAngle()
	{
		if (SeatAngleToGround-SavedSeatAngleToGround>2.0)
		{
			actTilt.moveOut();
		}
		else
		{
			if (SeatAngleToGround-SavedSeatAngleToGround<-2.0)
			{
				actTilt.moveIn();
			}
			else
			{
				actTilt.stop();
			}
		}

	}
	bool LegRestTouchGround()
	{
		return LegRestHeightToGround<30;
	}
	bool ReclineAngleInRange()
	{
		return ((BackAngleToGround<175)&&(BackAngleToGround>85));
	}
	bool LegRestInRange()
	{
		return ((LegRestHeightToGround>40)&&(LegRestAngleToGround>90));
	}
	void updateActPositionLimit()
	{
		if (actTilt.position>11)
		{
			actRecline.position_out_limit=100;
		}
		else
		{
			if (actTilt.position>6)
				actRecline.position_out_limit=75;
			else
				actRecline.position_out_limit=5;
		}

		if (actRecline.position<5)
		{
			actTilt.position_in_limit=0;
		}
		else
		{
			if(actRecline.position<75)
			{
				actTilt.position_in_limit=6;
			}
			else
			{
				actTilt.position_in_limit=11;
			}
		}
	}


}
