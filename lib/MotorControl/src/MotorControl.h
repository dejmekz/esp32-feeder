// Motor.h
#pragma once

#include <Arduino.h>


typedef void (*MotorControlCallback)(bool);


class MotorControl {
  public:
    MotorControl(uint8_t in1, uint8_t in2);
    void init();
    void start(uint8_t portions);
    void move(bool forward);
    void loop();
    void stop();
    void reset();

    // Callback setters
    void onMotorChange(MotorControlCallback callback);
    bool isRunning() const { return _portions > 0; }

  private:
    void callCallback(bool feeding);
    MotorControlCallback _onMotorChangeCallback = nullptr;
    unsigned long _lastMillis = 0;
    uint8_t _in1, _in2, _portions, _step;
};
