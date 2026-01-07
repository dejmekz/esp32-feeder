/**
 * @file Button.h
 * @author dejmekz
 * @brief Button control library with debouncing and edge detection
 * @version 0.2
 * @date 2022-12-05
 *
 * @copyright Copyright (c) 2021
 *
 */

#ifndef Button_h
#define Button_h

#include <Arduino.h>

/**
 * @brief Button class with built-in debouncing
 *
 * This class handles button input with debouncing to prevent false triggers
 * from mechanical switch bounce. It uses edge detection to return LOW only
 * once per button press.
 *
 * Usage:
 *   Button myButton(PIN, 100); // 100ms debounce delay
 *
 *   void loop() {
 *     if (myButton.read() == LOW) {
 *       // Button was just pressed
 *     }
 *   }
 */
class Button {
public:
  /**
   * @brief Construct a new Button object with custom debounce delay
   *
   * @param pin GPIO pin number
   * @param delay Debounce delay in milliseconds (default: 100ms recommended)
   */
  Button(uint8_t pin, long delay);

  /**
   * @brief Construct a new Button object with default 50ms debounce
   *
   * @param pin GPIO pin number
   */
  Button(uint8_t pin);

  /**
   * @brief Read the button state with debouncing
   *
   * Returns LOW only once when a stable button press is detected (falling edge).
   * Subsequent calls will return HIGH until the button is released and pressed again.
   *
   * @return uint8_t LOW if button just pressed, HIGH otherwise
   */
  uint8_t read();

  /**
   * @brief Get the last stable button state
   *
   * @return uint8_t Current stable state (HIGH or LOW)
   */
  uint8_t status();

private:
  void init(long delay);

  uint8_t _lastState;     // Last raw reading from the pin
  uint8_t _state;         // Current debounced state
  uint8_t _pin;           // GPIO pin number
  unsigned long _lastTime; // Last time the state changed
  long _delay;            // Debounce delay in milliseconds
};

#endif