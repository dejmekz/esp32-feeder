/**
 * @file Button.h
 * @author dejmekz
 * @brief Button control library
 * @version 0.1
 * @date 2022-12-05
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#ifndef Button_h
#define Button_h

#include <Arduino.h>


class Button {
public:
  Button(uint8_t pin, long delay);
  Button(uint8_t pin);
  uint8_t read();
  uint8_t status();

private:
  void init(long delay);

  uint8_t _lastState;
  uint8_t _state;
  uint8_t _pin;
  unsigned long _lastTime;
  long _delay;
};

#endif