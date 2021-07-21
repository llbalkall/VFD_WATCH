#ifndef CONTEXT_H
#define CONTEXT_H
#include "AbstractState.h"
#include "functions.h"
#include <VFDManager.h>
#include <SimpleTime.h>
#include <DS3231Manager.h>

class AbstractState;

class Commander
{
  /**
   * @var AbstractState A reference to the current state of the Commander.
   */
private:
  AbstractState *state_;
  char *days_of_week[7] = {"NN on", "tu E ", "UU Ed", "th u ", "Fr i ", "SA t ", "Su n "};
  

public:
  const short LOW_BATTERY_MESSAGE_DISPLAY_DURATION = 2000;
  const short LED_FLASH_INTERVAL = 150;
  const short MONTH_LENGTHS[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int setting_value = 0;
  bool stopwatch_running;
  Commander(AbstractState *state);
  ~Commander();
  ButtonManager buttonManager;
  VFDManager vfdManager = VFDManager();
  Time current_time;
  Time stop_watch_time;
  LEDs leds;
  TemperatureManager temperatureManager;
  DS3231Manager ds3231Manager;
  unsigned long current_millis;
  unsigned long wake_board_millis;

  void TransitionTo(AbstractState *state);
  void Update();

  void display_hour_minute();
  void display_date();
  void display_seconds();
  void display_day_of_week(byte day);
  void display_temperature();
  void display_stopwatch();
  void read_current_time();
  void flash_leds();
};

#endif // CONTEXT_H
