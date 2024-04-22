#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

class Utils
{
public:
  static void blinkLED(int pin, int _delayTime = 1000);
  static bool isButtonPressed(int pin);
  static bool isActiveOnLow(int pin);
  static bool isActiveOnHigh(int pin);
};

#endif
