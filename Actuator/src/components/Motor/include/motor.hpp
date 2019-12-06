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
#include <cmath>
#include <array>

namespace Motor {

    template <typename T>
    struct Vector2D {

        union {
            struct {
                T x;
                T y;
            };
            std::array<T, 2> data;
        };

        explicit Vector2D(T x, T y) : x(x), y(y) { }
        
        T Dot(const Vector2D& rhs) const {
            return x * rhs.x + y * rhs.y;
        }

        Vector2D operator-(const Vector2D& rhs) {
            return Vector2D(x - rhs.x, y - rhs.y);
        }

        Vector2D operator+(const Vector2D& rhs) { 
            return Vector2D(x + rhs.x, y + rhs.y);
        }

        Vector2D operator*(T scalar) { 
            return Vector2D(x * scalar, y * scalar);
        }

        Vector2D operator/(T scalar) { 
            if (scalar == 0.0) return *this;
            return Vector2D(x / scalar, y / scalar);
        }

        bool operator==(const Vector2D& rhs) {
        return (x == rhs.x && y == rhs.y);
        }

        T getLength() const {
            return sqrt(x * x + y * y);
        }

        void Normalize() {
            auto length = Length();
            if (length == T()) return;
            x /= length;
            y /= length;
        }

        Vector2D Normalized() {
            Vector2D result = *this;
            result.Normalize();
            return result;
        }
    };

    class Joystick {
        Vector2D<float> position = Vector2D<float>(0.0f, 0.0f);
        Vector2D<uint8_t> zero_position;
        Vector2D<uint8_t> min_position;
        Vector2D<uint8_t> max_position;
        Vector2D<uint8_t> zero_deadband; // deadband around zero

        template <size_t index>
        float convert(uint8_t raw_value) {
            float result = 0.0f;
            
            // clamp raw input between [min_position, max_position] for axis
            raw_value = std::max(std::min(raw_value, max_position.data[index]), min_position.data[index]);
            
            // If raw input within zero deadband, return 0;
            if (raw_value < (zero_position.data[index] + zero_deadband.data[index]) &&
                raw_value > (zero_position.data[index] - zero_deadband.data[index])) {
                // Do nothing
                // returning zero
            } else {

                // Check sign to see if raw value is above or below zero
                int sign = (raw_value > zero_position.data[index]) ? 1 : -1;

                auto diff = float(raw_value - (zero_position.data[index] + int(sign * zero_deadband.data[index])));

                result = diff * 100.0f / float(zero_position.data[index] - zero_deadband.data[index]);

                // [0, 120 <== deadband ==> 140, 225]
                // Case 1:
                //   raw_value = 115
                //   desired result = -5 / (120 - 0) (zero_position - deadband)
                // Case 2:
                //   raw_value = 145
                //   desired_value = 5 / (225 - 140)
            }

            return result;
        }

    public:
        explicit Joystick(
            const Vector2D<uint8_t>& zero_position = Vector2D<uint8_t>(127, 127),
            const Vector2D<uint8_t>& min_position = Vector2D<uint8_t>(0, 0),
            const Vector2D<uint8_t>& max_position = Vector2D<uint8_t>(255, 255),
            const Vector2D<uint8_t>& zero_deadband = Vector2D<uint8_t>(10, 10)) : 
            zero_position(zero_position),
            min_position(min_position), 
            max_position(max_position), 
            zero_deadband(zero_deadband) {}

        void convertRawInput(uint8_t x, uint8_t y) {
            position = Vector2D<float>(convert<0>(x), convert<1>(y));
        }

        Vector2D<float> getPosition() const {
            return position;
        }

    };

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
