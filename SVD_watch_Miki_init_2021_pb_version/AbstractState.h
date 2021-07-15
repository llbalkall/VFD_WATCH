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
  virtual void select_control_state() = 0;
};

#endif // ABSTRACTSTATE_H
