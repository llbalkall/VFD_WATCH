#ifndef CONTEXT_H
#define CONTEXT_H
#include "AbstractState.h"
#include "functions.h"
#include <VFDManager.h>
#include <SimpleTime.h>
#include <VFDManager.h>
#include <DS3231Manager.h>

class AbstractState;

class Context {
  /**
   * @var AbstractState A reference to the current state of the Context.
   */
 private:
  AbstractState *state_;
  char* days_of_week[7]= {"NN on", "tu E ", "UU Ed", "th u ", "Fr i ", "SA t ", "Su n "};
  
  
 public:
  const short MONTH_LENGTHS[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  int setting_value = 0;
  bool stopwatch_running;
  Context(AbstractState *state);
  ~Context();
  ButtonManager buttonManager;
  VFDManager vfdManager = VFDManager();
  Time current_time;
  Time stop_watch_time;
  TemperatureManager temperatureManager;
  DS3231Manager ds3231Manager;
  unsigned long current_millis;
  
  
  void TransitionTo(AbstractState *state);
  void Update();

  void display_hour_minute();
  void display_date();
  void display_seconds();
  void display_day_of_week(byte day);
  void display_temperature();
  void display_stopwatch();
};

#endif // CONTEXT_H
