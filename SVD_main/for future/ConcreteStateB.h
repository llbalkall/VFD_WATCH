#ifndef CONCRETESTATEB_H
#define CONCRETESTATEB_H
#include "AbstractState.h"

class ConcreteStateB : public AbstractState {
 public:
  void Handle1() override;
  void Handle2() override;
};

#endif // CONCRETESTATEB_H
