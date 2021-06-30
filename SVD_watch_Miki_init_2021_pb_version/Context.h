#ifndef CONTEXT_H
#define CONTEXT_H
#include "AbstractState.h"

class AbstractState;

class Context {
  /**
   * @var AbstractState A reference to the current state of the Context.
   */
 private:
  AbstractState *state_;

 public:
  Context(AbstractState *state);
  ~Context();
  /**
   * The Context allows changing the AbstractState object at runtime.
   */
  void TransitionTo(AbstractState *state);
  /**
   * The Context delegates part of its behavior to the current AbstractState object.
   */
  void Request1();

  void Request2();
};

#endif // CONTEXT_H
