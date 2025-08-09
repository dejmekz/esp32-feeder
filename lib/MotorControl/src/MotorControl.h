#ifndef _MOTORCONTROL_H
#define _MOTORCONTROL_H

#include <Arduino.h>
#ifndef USING_MOTOR
#include <Servo.h>
#endif

typedef void (*MotorControlCallback)(bool);

enum MotorDirection {
  MOTOR_FORWARD,
  MOTOR_BACKWARD,
  MOTOR_STOP
};


class MotorControl {
  public:
    #ifdef USING_MOTOR
    MotorControl(uint8_t in1, uint8_t in2);
    #else
    MotorControl(uint8_t in1);
    #endif
    void init();
    void start(uint8_t portions, unsigned long feedingTime);
    void move(MotorDirection direction);
    void loop();
    void stop();
    void reset();

    // Callback setters
    void onMotorChange(MotorControlCallback callback);
    bool isRunning() const { return _isRunning; }

  private:
    #ifndef USING_MOTOR
    Servo _servo;
    #endif
    void callCallback(bool feeding);
    MotorControlCallback _onMotorChangeCallback = nullptr;
    unsigned long _lastMillis, _feedingTime = 0;
    uint8_t _in1, _in2, _portions, _step = 0;
    bool _isRunning = false;
};

#endif