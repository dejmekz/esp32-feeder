// Motor.cpp
#include "MotorControl.h"

#ifdef USING_MOTOR
MotorControl::MotorControl(uint8_t in1, uint8_t in2)
    : _in1(in1), _in2(in2) {}
#else
MotorControl::MotorControl(uint8_t in1)
    : _in1(in1), _in2(0), _servo(Servo()) {}
#endif

void MotorControl::init()
{
    _step = 0;
    pinMode(_in1, OUTPUT);
#ifdef USING_MOTOR
    pinMode(_in2, OUTPUT);
#endif
    stop();
}

void MotorControl::move(MotorDirection direction)
{
#ifdef USING_MOTOR
    if (direction == MOTOR_FORWARD)
    {
        digitalWrite(_in1, HIGH);
        digitalWrite(_in2, LOW);
    }
    else if (direction == MOTOR_BACKWARD)
    {
        digitalWrite(_in1, LOW);
        digitalWrite(_in2, HIGH);
    }
    else
    {
        digitalWrite(_in1, LOW);
        digitalWrite(_in2, LOW); // Stop the motor
    }
#else
    if (direction == MOTOR_FORWARD)
    {
        _servo.write(_in1, 270);
    }
    else if (direction == MOTOR_BACKWARD)
    {
        _servo.write(_in1, 0);
    }
    else
    {
        _servo.write(_in1, 90); // Stop position
    }
#endif
}

void MotorControl::start(uint8_t portions, unsigned long feedingTime)
{
    _lastMillis = millis();
    _step = 0;
    _portions = portions;
    _feedingTime = feedingTime;
#ifndef USING_MOTOR
    _servo.attach(_in1, 1, 1000, 2000, false);
#endif

    callCallback(true);
    _isRunning = true;
}

void MotorControl::stop()
{
#ifdef USING_MOTOR
    move(MOTOR_STOP);
#else
    _servo.detach(_in1);
#endif
    callCallback(false);
    _isRunning = false;
}

void MotorControl::reset()
{
    _portions = 0;
    stop();
}

void MotorControl::onMotorChange(MotorControlCallback callback)
{
    _onMotorChangeCallback = callback;
}

void MotorControl::callCallback(bool feeding)
{
    {
    if (_onMotorChangeCallback)
        _onMotorChangeCallback(feeding);
    }
}

void MotorControl::loop()
{
    if (_portions == 0)
    {
        if (!_isRunning)
            return;
        stop();
        return;
    }

    unsigned long currentMillis = millis();
    unsigned long delayMillis = currentMillis - _lastMillis;

    if (_step == 0)
    {
        _lastMillis = currentMillis;
        move(MOTOR_BACKWARD);
        _step++;
    }
    else if (_step == 1 && delayMillis >= 500)
    {
        _lastMillis = currentMillis;
        move(MOTOR_STOP);
        _step++;
    }
    else if (_step == 2 && delayMillis >= 500)
    {
        _lastMillis = currentMillis;
        move(MOTOR_FORWARD); // Start moving forward
        _step++;
    }
    else if (_step == 3 && delayMillis >= _feedingTime)
    {
        _lastMillis = currentMillis;  // Reset timer for step 4
        move(MOTOR_STOP);
        _step++;
    }
    else if (_step == 4 && delayMillis >= 1000)
    {
        _lastMillis = currentMillis;
        _portions--;
        _step = 0; // Reset step to start the next dose
    }
}
