#ifndef WATERSENSOR_H
#define WATERSENSOR_H

#include <Arduino.h>
#include "Utils.h"
#include "Timer.h"

class LiquidSensor
{
private:
  int readInterval;
  long currentMillis;
  long previousMillis;
  float calibrationFactor;
  byte pulse1Sec;
  float flowRate;
  unsigned int flowMilliLitres;
  unsigned long totalMilliLitres;
  int waterFlowPin;
  int enableWaterFlowPin;

public:
  LiquidSensor(int flowPin, int enablePin, float _calibrationFactor = 4.5);

  void begin(int _readInterval = 1000);

  void update();

  float getFlowRate() const;

  unsigned long getTotalMilliLitres() const;

  float getTotalLitres() const;

private:
  static void IRAM_ATTR pulseCounter();
  static volatile byte pulseCount;
};

#endif
