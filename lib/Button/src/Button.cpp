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

Button::Button(uint8_t pin, long delay)
{
  _pin = pin;
  pinMode(_pin, INPUT);

  init(delay);
}

Button::Button(uint8_t pin) : Button(pin, 50) {}

void Button::init(long delay)
{
  _lastTime = 0;
  _delay = delay;
  _lastState = HIGH;
  _state = HIGH;
}

uint8_t Button::status()
{
  return _state;
}

uint8_t Button::read()
{
  unsigned long currentMillis = millis();
  uint8_t reading = digitalRead(_pin);

  // If the button state has changed (noise or actual press)
  if (reading != _lastState)
  {
    _lastTime = currentMillis; // Reset the debounce timer
    _lastState = reading;      // Update the last reading
  }

  // If enough time has passed and the state is stable
  if ((currentMillis - _lastTime) > _delay)
  {
    // If the stable state is different from our current state
    if (reading != _state)
    {
      _state = reading;

      // Return LOW only once when button is pressed (falling edge)
      if (_state == LOW)
      {
        return LOW;
      }
    }
  }

  // Return HIGH if not pressed or already processed
  return HIGH;
}