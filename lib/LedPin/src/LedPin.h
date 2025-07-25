/**
 * @file LedPin.h
 * @author meirarc
 * @brief Led control library
 * @version 0.1
 * @date 2021-10-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef LedPin_h
#define LedPin_h

#include <Arduino.h>


class LedPin {
public:  
  LedPin(uint8_t pin);
  LedPin(uint8_t pin, bool invert);
  void on();
  void off();  
  uint8_t status();
  void SetBlinkPeriod(long period);
  void blink();

private:
  uint8_t _pin;
  uint8_t _status;
  bool _blinkStatus;
  unsigned long _lastMillis;
  long _blinkPeriod;
  bool _invert;
};

#endif