#include "ConcreteStateB.h"
#include "ConcreteStateA.h"

void ConcreteStateA::Handle1() {
  {
    this->context_->TransitionTo(new ConcreteStateB);
  }
}

void ConcreteStateA::Handle2(){

}
