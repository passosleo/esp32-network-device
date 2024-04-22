#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

class Timer
{
public:
  Timer();
  void startTimer();
  void stopTimer();
  unsigned long getElapsedTime();
  bool isTimerRunning();

private:
  unsigned long startTime;
  unsigned long elapsedTime;
  bool timerRunning;
};

#endif
