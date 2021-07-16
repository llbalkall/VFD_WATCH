#ifndef CONCRETESTATEA_H
#define CONCRETESTATEA_H
#include "AbstractState.h"

class ConcreteStateA : public AbstractState {
 public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
};

class DisplayTime : public AbstractState {
 public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
};

class DisplayDate : public AbstractState {
 public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
};

class DisplayDayOfWeek : public AbstractState {
 public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
};

class DisplaySeconds : public AbstractState {
 public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
};

class DisplayTemperature : public AbstractState {
 public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
};

class StopWatch: public AbstractState{
  public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
};

class EnterSettings: public AbstractState{
  public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
};

class SettingNameHour: public AbstractState{
  public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
};

class SettingHour: public AbstractState{
  public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
}; 

class SettingNameMinute: public AbstractState{
  public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
};  

class SettingMinute: public AbstractState{
  public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
}; 
  
class SettingNameDayOfWeek: public AbstractState{
  public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
};   
 
class SettingDayOfWeek: public AbstractState{
  public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
};     

class SettingNameDayOfMonth: public AbstractState{
  public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
};   

class SettingDayOfMonth: public AbstractState{
  public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
};   

class SettingNameMonth: public AbstractState{
  public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
};  

class SettingMonth: public AbstractState{
  public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
};  
  
class SettingNameYear: public AbstractState{
  public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
}; 
  
class SettingYear: public AbstractState{
  public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
}; 
  
class SettingNameTemperature: public AbstractState{
  public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
}; 
  
class SettingTemperature: public AbstractState{
  public:
  void update_display() override;
  void first_pressed_and_released() override;
  void second_pressed_and_released() override;
}; 



#endif // CONCRETESTATEA_H