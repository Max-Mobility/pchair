#include "motor.hpp"
#include "I2C.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cmath>

namespace Motor {

float Lmotor_target = 0;
float Lmotor_curr = 0;
float Rmotor_target = 0;
float Rmotor_curr = 0;
float rampUp = 1.0;
float rampDown = 2.3;
float driveBackFactor = 0.4;

uint8_t forwardSpeedLimit = 70;
uint8_t rotationSpeedLimit = 17;
uint8_t phone_joystickX = 128;
uint8_t phone_joystickY = 128;

void setRotationSpeedLimit(float currentJoyStickThrowY) {
  uint8_t minRotationSpeedLimit = 3;
  uint8_t maxRotationSpeedLimit = 8;
  uint8_t rotationSpeedLimitRange = (maxRotationSpeedLimit - minRotationSpeedLimit);

  float factor = 1 - std::abs(currentJoyStickThrowY) / 100.0;
  rotationSpeedLimit = uint8_t(factor * rotationSpeedLimitRange + minRotationSpeedLimit);

  // Case 1: 75% of Y-direction joystick throw
  //   currentJoyStickThrowY = -75
  //   rotationSpeedLimit = (1 - 75 / 100) * 9 + 1 = 3.25
  // Case 2: 
  //   currentJoyStickThrowY = 0
  //   rotationSpeedLimit = (1 - 0) * 9 + 1 = 10 
  // Case 3:
  //   currentJoyStickThrow = 50
  //   rotationSpeedLimit = (1 - 50 / 100) * 9 + 1 = 4.5 + 1 = 5.5
}

void caculateSpeedLimit(void) {
  float speedSet = 0.5;

  switch (BLE::speedSettings) {
  case BLE::Speed_setting::speed_low:
    speedSet = 0.4; // 40% of full speed(15 mph)
    // rotationSpeedLimit = 3; // 3% X 15mph = 0.45mph
    rampUp = 0.75;
    rampDown = 1.3;
    break;
  case BLE::Speed_setting::speed_medium:
    speedSet = 0.7;
    // rotationSpeedLimit = 4; // 2; // used to be 4. Reduced after Mehdi's request 2019.12.03
    rampUp = 1.0;
    rampDown = rampUp; // 1.8; // used to be 2.3
    break;
  case BLE::Speed_setting::speed_high:
    speedSet = 1.0;
    // rotationSpeedLimit = 4; // 2; // used to be 4. Reduced after Mehdi's request 2019.12.03
    rampUp = 1.4;
    rampDown = rampUp; // 2.2; // used to be 3.3
    break;
  }

  // Actuator::speedLimit is a factor b/w (0.0, 1.0] that reduces the 
  // forwardSpeedLimit depending on actuator position, e.g., seating, standing etc.
  forwardSpeedLimit = uint8_t(speedSet * Actuator::speedLimit * 100);
  // rotationSpeedLimit = uint8_t(speedSet*Actuator::speedLimit*30);
}

void caculateMotorSpeed(float x, float y, Actuator::system_modes mode) {

  float left = 0;
  float right = 0;
  float factor = 1;
  if ((mode != Actuator::system_modes::DriveMode) &&
      (mode != Actuator::system_modes::PhoneControlMode)) {
    //			printf("joystick error!\n");
    // Lmotor_curr = caculateSpeedRamp(Lmotor_curr, 0);
    // Rmotor_curr = caculateSpeedRamp(Rmotor_curr, 0);
    // Will/Pranav: Commented out the above lines because:
    // When the steering wheel goes out of range, we want
    // the chair to come to a full stop quickly
    // So, we're setting the left and right speeds to 0
    // as seen below:

    Lmotor_curr = 0.0f;
    Rmotor_curr = 0.0f;

    SerialTask::leftSpeed[1] = char(0);
    SerialTask::rightSpeed[1] = char(0);
    SerialTask::leftSpeed[0] = char(Lmotor_curr);
    SerialTask::rightSpeed[0] = char(Rmotor_curr);
  } else {
    caculateSpeedLimit();
    setRotationSpeedLimit(y);
    if (y < 0) // driving backward
    {
      left = (y * forwardSpeedLimit * 0.4 / 100.0 +
              x * rotationSpeedLimit / 100.0);  // backward speed is 40% of farward speed
      right = (y * forwardSpeedLimit * 0.4 / 100.0 -
               x * rotationSpeedLimit / 100.0);
    } else // driving forward
    {
      left = (y * forwardSpeedLimit / 100.0 + x * rotationSpeedLimit / 100.0);
      right = (y * forwardSpeedLimit / 100.0 - x * rotationSpeedLimit / 100.0);
    }

    // At this point, left and right represent actual motor speed
    // in percentages that will be sent as motor control commands to the chair

    //			printf("joyx=%f,joyY=%f,left=%f,right=%f \n
    //",x,y,left,right);
    if (abs(left) > forwardSpeedLimit) {
      factor = forwardSpeedLimit / float(abs(left));
    }
    if (abs(right) > forwardSpeedLimit) {
      factor = forwardSpeedLimit / float(abs(right));
    }

    // Let's say forwardSpeedLimit = 15%
    // left = 17% = 0.17
    // right = 13% = 0.13
    // Then, after the following lines, 
    // newLeft = 0.17 * 0.15 / 0.17 = 0.15
    // newRight = -1 * 0.13 * 0.15 / 0.17 = -0.11

    left = left * factor;
    right = -1 * right * factor; // motor mount reversed.

    // newLeft = 0.15 is the targetSpeed
    // newRight = -0.11 is the targetSpeed
    // By the time we get here, 
    // the target speeds we send to calculateSpeedRamp 
    // are no greater than forward speed limit

    Lmotor_curr = caculateSpeedRamp(Lmotor_curr, left);
    Rmotor_curr = caculateSpeedRamp(Rmotor_curr, right);

    SerialTask::leftSpeed[1] = char(left);
    SerialTask::rightSpeed[1] = char(right);
    SerialTask::leftSpeed[0] = char(Lmotor_curr);
    SerialTask::rightSpeed[0] = char(Rmotor_curr);
    //			printf("left=%f,right=%f,joyx=%f,joyY=%f\n",Lmotor_curr,Rmotor_curr,x,y
    //);
  }
}

float converterJoystickReadingX(uint8_t r) {
  // [27, 120 127 140, 225]
  // Result is b/w -100 and 100
  float x = 0;
  if (r > joyStickXMax) {
    r = joyStickXMax;
  }
  if (r < joyStickXMin) {
    r = joyStickXMin;
  }
  if (r > joyStickXZeroMax) {
    x = ((r - joyStickXZeroMax) * 100.0 / (joyStickXMax - joyStickXZeroMax));
  } else {
    if (r < joyStickXZeroMin) {
      x = ((r - joyStickXZeroMin) * 100.0 / (joyStickXZeroMin - joyStickXMin));
    } else
      x = 0;
  }
  return x;
}

float converterJoystickReadingY(uint8_t r) {
  // Result is b/w -100 and 100
  float y = 0;
  if (r > joyStickYMax) {
    r = joyStickYMax;
  }
  if (r < joyStickYMin) {
    r = joyStickYMin;
  }
  if (r > joyStickYZeroMax) {
    y = ((r - joyStickYZeroMax) * 100.0 / (joyStickYMax - joyStickYZeroMax));
  } else {
    if (r < joyStickYZeroMin) {
      y = ((r - joyStickYZeroMin) * 100.0 / (joyStickYZeroMin - joyStickYMin));
    } else
      y = 0;
  }
  return y;
}

float converterJoystickReadingPhone(uint8_t r) {
  float x = 0;
  if (r > 148) {
    x = ((r - 148) * 100.0 / (255 - 148));
  } else {
    if (r < 108) {
      x = ((r - 108) * 100.0 / 108);
    } else
      x = 0;
  }
  return x;
}

float caculateSpeedRamp(float currSpeed, float targetSpeed) {
  auto result = currSpeed;
  // Find signed diff between currSpeed and targetSpeed
  auto diff = targetSpeed - currSpeed;
  auto rampFactor = rampDown;

  if (std::abs(diff) < rampFactor) {
    // Set to target speed
    result = targetSpeed;
  } else {
    // We need to ramp
    if (diff < 0.0f) {
      result -= rampFactor;
    } else {
      result += rampFactor;
    }
  }
  return result;

  // Sanity checks
  // Case 1: currSpeed = 2, targetSpeed = 5
  //    diff = 3
  //    rampDown = 2
  //    result = currSpeed + rampFactor = 4
  //
  // Case 2: currSpeed = -2, targetSpeed = -5
  //    diff = -3
  //    rampDown = 2
  //    result = currSpeed - rampFactor = -4
  //
  // Case 3: currSpeed = -5, targetSpeed = 5
  //    diff = 5 -  (-5) = 10
  //    rampDown = 2
  //    result = currSpeed + rampFactor = -5 + 2 = -3
  //
  // Case 4: currSpeed = 5, targetSpeed = -5
  //    diff = -5 - 5 = -10
  //    rampDown = 2
  //    result = currSpeed - rampFactor = 5 - 2 = 3
}

// Takes joystick value and performs slerp
// from our current/saved state value to new value of what was
// read from the joystick
// 
// All values are in % range [-100, 100]
Vector2f Slerp(Vector2f start, Vector2f end, float percent)
{
     // Dot product - the cosine of the angle between 2 vectors.
     float dot = (start / 100.0f).Dot(end / 100.0f);
     // Clamp it to be in the range of Acos()
     // This may be unnecessary, but floating point
     // precision can be a fickle mistress.
     dot = std::max(std::min(dot, 1.0f), -1.0f); 
     // Acos(dot) returns the angle between start and end,
     // And multiplying that by percent returns the angle between
     // start and the final result.
     float theta = acos(dot) * percent;
     Vector2f RelativeVec = end - start * dot;
     // RelativeVec.Normalize();
     // Orthonormal basis
     // The final result.
     return ((start * cos(theta)) + (RelativeVec * sin(theta))) * 100.0f;

    // Case 1: {0.5, 0.5} to {1.0, 0.0}
    //   dot = 0.5
    //   acos(dot) = 1.047
    //   percent = 50%
    //   theta = 0.523
    //   RelativeVec = {1.0, 0.0} - {0.25, 0.25} = {0.75, -0.25}
    //   Result = {0.433, 0.433} + {0.375, -0.125} = ~ { 0.8, 0.31 }
}

void taskFunction(void *pvParameter) {
  float joyXReading = 0, joyYReading = 0;
  // Vector2f interp(0.0f, 0.0f);

  while (true) {
    if (Actuator::systemMode == Actuator::system_modes::PhoneControlMode) {
      joyXReading = converterJoystickReadingPhone(phone_joystickX);
      joyYReading = converterJoystickReadingPhone(phone_joystickY);

    } else {
      joyXReading = converterJoystickReadingX(I2C::joystickX);
      joyYReading = converterJoystickReadingY(I2C::joystickY);
    }

    // interp = Slerp(interp, Vector2f{joyXReading, joyYReading}, 0.05);

    caculateMotorSpeed(joyXReading, joyYReading, Actuator::systemMode);
    vTaskDelay((50 * (1)) / portTICK_RATE_MS);
  }
}

} // namespace Motor
