#ifndef CONCRETESTATEA_H
#define CONCRETESTATEA_H
#include "AbstractState.h"

class ConcreteStateA : public AbstractState {
 public:
  void Handle1() override;
  void Handle2() override;
};

#endif // CONCRETESTATEA_H
