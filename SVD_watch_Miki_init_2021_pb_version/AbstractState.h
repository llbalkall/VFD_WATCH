#ifndef ABSTRACTSTATE_H
#define ABSTRACTSTATE_H
#include "Context.h"


class Context;

class AbstractState
{
    /**
    * @var Context
    */
    protected:
  Context *context_;

    public:
  virtual ~AbstractState();

  void set_context(Context *context);
  virtual void update_display() = 0;
  virtual void first_pressed_and_released() = 0;
  virtual void second_pressed_and_released() = 0;
  //virtual void power_down() = 0;
  void first_held();
  void second_held();
  void both_held();
};



#endif // ABSTRACTSTATE_H
