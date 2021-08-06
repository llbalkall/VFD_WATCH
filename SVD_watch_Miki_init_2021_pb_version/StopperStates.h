#ifndef STOPPERSTATES_H
#define STOPPERSTATES_H
#include "AbstractState.h"

class StopWatchRunning : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class StopWatchStopped : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class StopWatchNulled : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

#endif
