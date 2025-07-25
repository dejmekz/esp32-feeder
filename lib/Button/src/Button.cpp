/**
 * @file Button.cpp
 * @author dejmekz
 * @brief Button control library
 * @version 0.1
 * @date 2022-12-05
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "Button.h"

Button::Button(uint8_t pin, long delay) {
  _pin = pin;
  pinMode(_pin, INPUT);

  init(delay);
}

Button::Button(uint8_t pin) {
  _pin = pin;
  pinMode(_pin, INPUT);

  init(50);
}

void Button::init(long delay) {
  _lastTime = 0;
  _delay = delay;
  _lastState = HIGH;  
}

uint8_t Button::status() {
  return _lastState;
}

uint8_t Button::read() {
  unsigned long currentMillis = millis();
  uint8_t reading = digitalRead(_pin);
  if (reading != _lastState)
  {
    _lastTime = millis();
    _state = HIGH;
  }    

  if ((currentMillis - _lastTime) > _delay) {
    // if the button state has changed:
    if (reading != _state) {
      _state = reading;
    }
  }

  // save the reading. Next time through the loop, it'll be the _lastState:
  _lastState = reading;
  return _state;
}