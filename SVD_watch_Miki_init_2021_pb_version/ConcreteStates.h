#ifndef CONCRETESTATEA_H
#define CONCRETESTATEA_H
#include "AbstractState.h"

class ConcreteStateA : public AbstractState {
 public:
  void select_control_state() override;
};

class DisplayTime : public AbstractState {
 public:
  void select_control_state() override;
};

class DisplayDate : public AbstractState {
 public:
  void select_control_state() override;
};

class DisplayDayOfWeek : public AbstractState {
 public:
  void select_control_state() override;
};

class DisplaySeconds : public AbstractState {
 public:
  void select_control_state() override;
};

class DisplayTemperature : public AbstractState {
 public:
  void select_control_state() override;
};

#endif // CONCRETESTATEA_H