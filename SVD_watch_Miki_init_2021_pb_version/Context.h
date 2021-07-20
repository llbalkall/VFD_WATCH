#ifndef CONTEXT_H
#define CONTEXT_H
#include "AbstractState.h"
#include "functions.h"
#include <VFDManager.h>
#include <SimpleTime.h>

class AbstractState;

class Context {
  /**
   * @var AbstractState A reference to the current state of the Context.
   */
 private:
  AbstractState *state_;
  char* days_of_week[7]= {"NN on", "tu E ", "UU Ed", "th u ", "Fr i ", "SA t ", "Su n "};
 public:
  Context(AbstractState *state);
  ~Context();
  ButtonManager buttonManager;
  VFDManager vfdManager = VFDManager();
  Time current_time;
  TemperatureManager temperatureManager;
  unsigned long current_millis;
  /**
   * The Context allows changing the AbstractState object at runtime.
   */
  void TransitionTo(AbstractState *state);
  /**
   * The Context delegates part of its behavior to the current AbstractState object.
   */
  void Update();

  void Context::display_hour_minute();
  void Context::display_date();
  void Context::display_seconds();
  void Context::display_day_of_week(byte day);
  void Context::display_temperature();
};

#endif // CONTEXT_H
