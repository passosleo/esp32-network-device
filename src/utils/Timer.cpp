#include "Timer.h"

Timer::Timer()
{
  timerRunning = false;
  startTime = 0;
  elapsedTime = 0;
}

void Timer::startTimer()
{
  startTime = millis();
  timerRunning = true;
}

void Timer::stopTimer()
{
  elapsedTime = millis() - startTime;
  timerRunning = false;
}

unsigned long Timer::getElapsedTime()
{
  if (timerRunning)
  {
    return millis() - startTime;
  }
  else
  {
    return elapsedTime;
  }
}

bool Timer::isTimerRunning()
{
  return timerRunning;
}
