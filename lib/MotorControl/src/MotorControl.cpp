 // Motor.cpp
#include "MotorControl.h"

MotorControl::MotorControl(uint8_t in1, uint8_t in2)
    : _in1(in1), _in2(in2) {}

void MotorControl::init() {
    _step = 0;
    _portions = 0;
    pinMode(_in1, OUTPUT);
    pinMode(_in2, OUTPUT);
    stop();
}

void MotorControl::move(bool forward)
{
  if (forward) {
    digitalWrite(_in1, HIGH);
    digitalWrite(_in2, LOW);
  } else {
    digitalWrite(_in1, LOW);
    digitalWrite(_in2, HIGH);
  }
}

void MotorControl::start(uint8_t portions) {
    _lastMillis = millis();
    _step = 0;
    _portions = portions;

    callCallback(true);
}

void MotorControl::stop() {
    digitalWrite(_in1, LOW);
    digitalWrite(_in2, LOW);

    callCallback(false);
}

void MotorControl::reset() {
    _portions = 0;
    stop();    
}

void MotorControl::onMotorChange(MotorControlCallback callback)
{
    _onMotorChangeCallback = callback;
}

void MotorControl::callCallback(bool feeding) {
        if (_onMotorChangeCallback) {
            _onMotorChangeCallback(feeding);
        }
    }


void MotorControl::loop() {
    if (_portions == 0) return;

    unsigned long currentMillis = millis();
    unsigned long delayMillis = currentMillis - _lastMillis;

    if (_step == 0) {
        _lastMillis = currentMillis;
        move(false);
        _step++;
    } else if (_step == 1 && delayMillis >= 500) {
        _lastMillis = currentMillis;
        stop();
        _step++;
    } else if (_step == 2 && delayMillis >= 500) {
        _lastMillis = currentMillis;
        move(true); // Start moving forward
        _step++;
    } else if (_step == 3 && delayMillis >= 2000) {
        stop(); // Stop the motor when done
        _step++;
    } else if (_step == 4 && delayMillis >= 1000) {
        _lastMillis = currentMillis;
        _portions--;
        _step = 0; // Reset step to start the next dose
    }
}
