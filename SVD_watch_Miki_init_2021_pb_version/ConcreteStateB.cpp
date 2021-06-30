#include "ConcreteStateB.h"
#include "ConcreteStateA.h"

void ConcreteStateB::Handle1() {
}

void ConcreteStateB::Handle2() {
  this->context_->TransitionTo(new ConcreteStateA);
}
