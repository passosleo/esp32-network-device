#include "LiquidSensor.h"

volatile byte LiquidSensor::pulseCount = 0;

LiquidSensor::LiquidSensor(int flowPin, int enablePin, float _calibrationFactor)
    : calibrationFactor(_calibrationFactor), waterFlowPin(flowPin), enableWaterFlowPin(enablePin) {}

void LiquidSensor::begin(int _readInterval)
{
  pinMode(waterFlowPin, INPUT_PULLUP);
  pinMode(enableWaterFlowPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(waterFlowPin), pulseCounter, FALLING);
  readInterval = _readInterval;
  pulseCount = 0;
  flowRate = 0.0;
  flowMilliLitres = 0;
  totalMilliLitres = 0;
  Timer.startTimer();
}

void LiquidSensor::update()
{
  if (Utils::isActiveOnLow(enableWaterFlowPin))
  {
    if (Timer.getElapsedTime() > readInterval)
    {
      pulse1Sec = pulseCount;
      pulseCount = 0;
      flowRate = ((1000.0 / Timer.getElapsedTime()) * pulse1Sec) / calibrationFactor;
      Timer.startTimer();
      flowMilliLitres = (flowRate / 60) * 1000;
      totalMilliLitres += flowMilliLitres;
    }
  }
  else
  {
    totalMilliLitres = 0;
    flowRate = 0;
  }
}

float LiquidSensor::getFlowRate() const
{
  return flowRate;
}

unsigned long LiquidSensor::getTotalMilliLitres() const
{
  return totalMilliLitres;
}

float LiquidSensor::getTotalLitres() const
{
  return totalMilliLitres / 1000.0;
}

void LiquidSensor::pulseCounter()
{
  pulseCount++;
}
