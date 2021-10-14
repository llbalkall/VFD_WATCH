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

class BackToTheFutureAnimation : public AbstractState
{
  int CHECKPOINT_1 = 1000;
  int CHECKPOINT_2 = 2000;
  int CHECKPOINT_3 = 3000;
  int CHECKPOINT_4 = 4000;
  int CHECKPOINT_5 = 5000;
  int to_88 = 4100;
  const int SINGLE_ANIMATION_NUMBER = 4;
  const int DOUBLE_ANIMATION_NUMBER = 2;
  const int FULL_ANIMATION_NUMBER = 2;
  int single_animation_counter = 0;
  int double_animation_counter = 0;
  int full_animation_counter = 0;
  int single_animation_duration = 250;
  int double_animation_duration = 250;
  int full_animation_duration = 375;
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
  long animation_millis();
  void next_state();
  void reset_animation_time();
  int single_line_animation();  //=
  int double_line_animation();  //==
  int full_line_animation();    //====
};

#endif
