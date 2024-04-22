#include "Utils.h"

void Utils::blinkLED(int pin, int _delayTime)
{
  digitalWrite(pin, LOW);
  delay(_delayTime);
  digitalWrite(pin, HIGH);
  delay(_delayTime);
}

bool Utils::isButtonPressed(int pin)
{
  return digitalRead(pin) == LOW;
}

bool Utils::isActiveOnLow(int pin)
{
  return digitalRead(pin) == LOW;
}

bool Utils::isActiveOnHigh(int pin)
{
  return digitalRead(pin) == HIGH;
}
