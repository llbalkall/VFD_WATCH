#ifndef ABSTRACTSTATE_H
#define ABSTRACTSTATE_H
#include "Commander.h"

class Commander;

class AbstractState
{
  /**
    * @var Commander
    */
protected:
  Commander *commander;

public:
  virtual ~AbstractState();

  void set_context(Commander *commander);
  virtual void update_display() = 0;
  virtual void top_pressed_and_released() = 0;
  virtual void bottom_pressed_and_released() = 0;
  //virtual void power_down() = 0;
  void top_held();
  void bottom_held();
  void both_held();
};

#endif // ABSTRACTSTATE_H
