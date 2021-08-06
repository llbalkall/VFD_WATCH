#ifndef CONCRETESTATEA_H
#define CONCRETESTATEA_H
#include "AbstractState.h"
#include "StopperStates.h"

class ConcreteStateA : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class DisplayTime : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class DisplayDate : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class DisplayDayOfWeek : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class DisplaySeconds : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class DisplayTemperature : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class EnterSettings : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class SettingNameAlarm : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class SettingAlarmMode : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class SettingAlarmHour : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};
class SettingAlarmMinute : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class SettingNameTime : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class SettingHour : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class SettingMinute : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class SettingNameDate : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class SettingMonth : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class SettingDayOfMonth : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class SettingNameDayOfWeek : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class SettingDayOfWeek : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class SettingNameYear : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class SettingYear : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class SettingNameTemperature : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class SettingTemperature : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class Alarm : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class SnoozeMessage : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class SettingPartyModeName : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};

class SettingPartyMode : public AbstractState
{
public:
  void update_display() override;
  void top_pressed_and_released() override;
  void bottom_pressed_and_released() override;
};
#endif // CONCRETESTATEA_H
