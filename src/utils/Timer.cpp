#include "Timer.h"

TimerClass Timer;

TimerClass::TimerClass()
{
  timerRunning = false;
  startTime = 0;
  elapsedTime = 0;
}

void TimerClass::startTimer()
{
  startTime = millis();
  timerRunning = true;
}

void TimerClass::stopTimer()
{
  elapsedTime = millis() - startTime;
  timerRunning = false;
}

unsigned long TimerClass::getElapsedTime()
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

bool TimerClass::isTimerRunning()
{
  return timerRunning;
}
