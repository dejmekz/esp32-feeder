/**
 * @file Led.cpp
 * @author meirarc
 * @brief Led control methods
 * @version 0.1
 * @date 2021-10-11
 * 
 * @copyright Copyright (c) 2021
 * 
 */
#include "LedPin.h"

/**
 * @brief Construct a new Led:: Led object
 * 
 * @param pin 
 */
LedPin::LedPin(uint8_t pin) : LedPin(pin, false) {}

/**
 * @brief Construct a new Led:: Led object
 * 
 * @param pin 
 * @param blinkPeriod 
 */
LedPin::LedPin(uint8_t pin, bool inverted) {
  _pin = pin;
  pinMode(_pin, OUTPUT);

  _lastMillis = 0;
  _blinkPeriod = 1000;
  _status = inverted ? LOW : HIGH;
  _blinkStatus = inverted;
  _invert = inverted;
  digitalWrite(_pin, _status);
}

/**
 * @brief on() method. Set pin as HIGH
 * 
 */
void LedPin::on() {
  _status = _invert ? LOW : HIGH;
  digitalWrite(_pin, _status);
}

/**
 * @brief off() method. Set pin as LOW
 * 
 */
void LedPin::off() {
  _status = _invert ? HIGH : LOW;

  digitalWrite(_pin, _status);
}

/**
 * @brief status() method. Return true for pin as HIGH and false for the pin LOW
 * 
 * @return true 
 * @return false 
 */

uint8_t LedPin::status() {
  return _status;
}


void LedPin::SetBlinkPeriod(long period) {
  _blinkPeriod = period;
}


/**
 * @brief blink(blinkPeriod). blink the led based on the blinkPeriod in milliseconds.
 * 
 * @param blinkPeriod 
 */
void LedPin::blink() {
  unsigned long currentMillis = millis();
  unsigned long elapsed = currentMillis - _lastMillis;
  if (elapsed >= _blinkPeriod) {
    if (_blinkStatus)
    {
      off();
      _blinkStatus = false;
    }
    else
    {
      on();
      _blinkStatus = true;
    }

    _lastMillis = currentMillis;
  }
}