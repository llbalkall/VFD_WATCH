#include "ConcreteStates.h"

void ConcreteStateA::update_display()
{
}

void DisplayTime::update_display()
{
  this->commander->display_hour_minute();
}

void DisplayTime::top_pressed_and_released()
{
  this->commander->TransitionTo(new DisplayDate);
}

void DisplayTime::bottom_pressed_and_released()
{
  this->commander->TransitionTo(new DisplayDayOfWeek);
}

void DisplayDate::update_display()
{
  this->commander->display_date();
}

void DisplayDate::top_pressed_and_released()
{
  this->commander->TransitionTo(new DisplaySeconds);
}

void DisplayDate::bottom_pressed_and_released()
{
  this->commander->TransitionTo(new DisplayDayOfWeek);
}

void DisplaySeconds::update_display()
{
  this->commander->display_seconds();
}

void DisplaySeconds::top_pressed_and_released()
{
  this->commander->TransitionTo(new DisplayTime);
}

void DisplaySeconds::bottom_pressed_and_released()
{
  this->commander->TransitionTo(new DisplayDayOfWeek);
}

void DisplayDayOfWeek::update_display()
{
  this->commander->display_day_of_week(this->commander->current_time.dayOfWeek);
}

void DisplayDayOfWeek::top_pressed_and_released()
{
  this->commander->TransitionTo(new DisplayTime);
}

void DisplayDayOfWeek::bottom_pressed_and_released()
{
  this->commander->TransitionTo(new DisplayTemperature);
}

void DisplayTemperature::update_display()
{
  this->commander->display_temperature();
}

void DisplayTemperature::top_pressed_and_released()
{
  this->commander->TransitionTo(new DisplayTime);
}

void DisplayTemperature::bottom_pressed_and_released()
{
  this->commander->TransitionTo(new DisplayTime);
}

void StopWatch::update_display()
{
  this->commander->display_stopwatch();
}

void StopWatch::top_pressed_and_released()
{
  this->commander->TransitionTo(new DisplayTime);
}

void StopWatch::bottom_pressed_and_released()
{
  if (this->commander->stopwatch_running)
  {
    this->commander->stopwatch_running = false;
    this->commander->stop_watch_time.dayOfMonth = this->commander->current_time.dayOfMonth - this->commander->stop_watch_time.dayOfMonth;
    this->commander->stop_watch_time.hour = this->commander->current_time.hour - this->commander->stop_watch_time.hour;
    this->commander->stop_watch_time.minute = this->commander->current_time.minute - this->commander->stop_watch_time.minute;
    this->commander->stop_watch_time.second = this->commander->current_time.second - this->commander->stop_watch_time.second;
  }
  else if (this->commander->stop_watch_time.second == 0 && this->commander->stop_watch_time.minute == 0 && this->commander->stop_watch_time.hour == 0)
  {
    this->commander->stopwatch_running = true;
    this->commander->stop_watch_time.second = this->commander->current_time.second;
    this->commander->stop_watch_time.minute = this->commander->current_time.minute;
    this->commander->stop_watch_time.hour = this->commander->current_time.hour;
    this->commander->stop_watch_time.dayOfMonth = this->commander->current_time.dayOfMonth;
  }
  else
  {
    this->commander->stop_watch_time.second = 0;
    this->commander->stop_watch_time.minute = 0;
    this->commander->stop_watch_time.hour = 0;
    this->commander->stop_watch_time.dayOfMonth = 0;
  }
}

void EnterSettings::update_display()
{
  this->commander->vfdManager.update_char_array("SE t ");
}

void EnterSettings::top_pressed_and_released()
{
  this->commander->TransitionTo(new DisplayTime);
}

void EnterSettings::bottom_pressed_and_released()
{
  this->commander->TransitionTo(new SettingNameHour);
}

void SettingNameHour::update_display()
{
  this->commander->vfdManager.update_char_array("Ho ur");
}

void SettingNameHour::top_pressed_and_released()
{
  this->commander->setting_value = this->commander->current_time.hour;
  this->commander->TransitionTo(new SettingHour);
}

void SettingNameHour::bottom_pressed_and_released()
{
  this->commander->TransitionTo(new SettingNameMinute);
}

void SettingHour::update_display()
{
  this->commander->vfdManager.update_char_array(this->commander->setting_value / 10,
                                                this->commander->setting_value % 10,
                                                ' ',
                                                ' ',
                                                ' ');
}

void SettingHour::top_pressed_and_released()
{
  this->commander->TransitionTo(new SettingNameMinute);
}

void SettingHour::bottom_pressed_and_released()
{
  this->commander->setting_value += 1;
  if (this->commander->setting_value == 24)
    this->commander->setting_value = 0;
  this->commander->ds3231Manager.set_hour(this->commander->setting_value, this->commander->current_time);
}

void SettingNameMinute::update_display()
{
  this->commander->vfdManager.update_char_array("NN in");
}

void SettingNameMinute::top_pressed_and_released()
{
  this->commander->TransitionTo(new SettingMinute);
  this->commander->setting_value = this->commander->current_time.minute;
}

void SettingNameMinute::bottom_pressed_and_released()
{
  this->commander->vfdManager.displayed_characters[3] = 'o';
  this->commander->TransitionTo(new SettingNameDayOfWeek);
}

void SettingMinute::update_display()
{
  this->commander->vfdManager.update_char_array(' ',
                                                ' ',
                                                ' ',
                                                this->commander->setting_value / 10,
                                                this->commander->setting_value % 10);
}

void SettingMinute::top_pressed_and_released()
{
  this->commander->TransitionTo(new SettingNameDayOfWeek);
}

void SettingMinute::bottom_pressed_and_released()
{
  this->commander->setting_value += 1;
  if (this->commander->setting_value == 60)
    this->commander->setting_value = 0;
  this->commander->ds3231Manager.set_minute(this->commander->setting_value, this->commander->current_time);
}

void SettingNameDayOfWeek::update_display()
{
  this->commander->vfdManager.update_char_array("do UU");
}

void SettingNameDayOfWeek::top_pressed_and_released()
{
  this->commander->setting_value = this->commander->current_time.dayOfWeek;
  this->commander->TransitionTo(new SettingDayOfWeek);
}

void SettingNameDayOfWeek::bottom_pressed_and_released()
{
  this->commander->TransitionTo(new SettingNameDayOfMonth);
}

void SettingDayOfWeek::update_display()
{
  this->commander->display_day_of_week(this->commander->setting_value);
}

void SettingDayOfWeek::top_pressed_and_released()
{
  this->commander->TransitionTo(new SettingNameDayOfMonth);
}

void SettingDayOfWeek::bottom_pressed_and_released()
{
  this->commander->setting_value += 1;
  if (this->commander->setting_value == 8)
    this->commander->setting_value = 1;
  this->commander->ds3231Manager.set_dayOfWeek(this->commander->setting_value, this->commander->current_time);
}

void SettingNameDayOfMonth::update_display()
{
  this->commander->vfdManager.update_char_array("do NN");
}

void SettingNameDayOfMonth::top_pressed_and_released()
{
  this->commander->setting_value = this->commander->current_time.dayOfMonth;
  this->commander->TransitionTo(new SettingDayOfMonth);
}

void SettingNameDayOfMonth::bottom_pressed_and_released()
{
  this->commander->TransitionTo(new SettingNameMonth);
}

void SettingDayOfMonth::update_display()
{
  this->commander->vfdManager.update_char_array(' ',
                                                ' ',
                                                ' ',
                                                this->commander->setting_value / 10,
                                                this->commander->setting_value % 10);
}

void SettingDayOfMonth::top_pressed_and_released()
{
  this->commander->TransitionTo(new SettingNameMonth);
}

void SettingDayOfMonth::bottom_pressed_and_released()
{
  this->commander->setting_value += 1;
  if (this->commander->setting_value == (this->commander->MONTH_LENGTHS[this->commander->current_time.month] + 1))
  {
    this->commander->setting_value = 1;
  }
  // Leap day condition
  if (this->commander->current_time.year % 4 == 0 &&
      this->commander->current_time.month == 2 &&
      this->commander->setting_value == 29)
  {
    this->commander->setting_value = 1;
  }
  this->commander->ds3231Manager.set_dayOfMonth(this->commander->setting_value, this->commander->current_time);
}

void SettingNameMonth::update_display()
{
  this->commander->vfdManager.update_char_array("NN on");
}

void SettingNameMonth::top_pressed_and_released()
{
  this->commander->setting_value = this->commander->current_time.month;
  this->commander->TransitionTo(new SettingMonth);
}

void SettingNameMonth::bottom_pressed_and_released()
{
  this->commander->TransitionTo(new SettingNameYear);
}

void SettingMonth::update_display()
{
  this->commander->vfdManager.update_char_array(this->commander->setting_value / 10,
                                                this->commander->setting_value % 10,
                                                ' ',
                                                ' ',
                                                ' ');
}

void SettingMonth::top_pressed_and_released()
{
  this->commander->TransitionTo(new SettingNameYear);
}

void SettingMonth::bottom_pressed_and_released()
{
  this->commander->setting_value += 1;
  if (this->commander->setting_value == 13)
    this->commander->setting_value = 1;
  this->commander->ds3231Manager.set_month(this->commander->setting_value, this->commander->current_time);
}

void SettingNameYear::update_display()
{
  this->commander->vfdManager.update_char_array("YE Ar");
}

void SettingNameYear::top_pressed_and_released()
{
  this->commander->setting_value = this->commander->current_time.year;
  this->commander->TransitionTo(new SettingYear);
}

void SettingNameYear::bottom_pressed_and_released()
{
  this->commander->TransitionTo(new SettingNameTemperature);
}

void SettingYear::update_display()
{
  this->commander->vfdManager.update_char_array('2',
                                                '0',
                                                ' ',
                                                this->commander->setting_value / 10,
                                                this->commander->setting_value % 10);
}

void SettingYear::top_pressed_and_released()
{
  this->commander->TransitionTo(new SettingNameTemperature);
}

void SettingYear::bottom_pressed_and_released()
{
  this->commander->setting_value += 1;
  if (this->commander->setting_value == 100)
    this->commander->setting_value = 20;
  this->commander->ds3231Manager.set_year(this->commander->setting_value, this->commander->current_time);
}

void SettingNameTemperature::update_display()
{
  this->commander->vfdManager.update_char_array("tE nn");
}

void SettingNameTemperature::top_pressed_and_released()
{
  this->commander->TransitionTo(new SettingTemperature);
}

void SettingNameTemperature::bottom_pressed_and_released()
{
  this->commander->TransitionTo(new SettingNameHour);
}

void SettingTemperature::update_display()
{
  this->commander->vfdManager.update_char_array(' ',
                                                ' ',
                                                ' ',
                                                '*',
                                                this->commander->temperatureManager.temperature_unit);
}

void SettingTemperature::top_pressed_and_released()
{
  this->commander->TransitionTo(new SettingNameHour);
}

void SettingTemperature::bottom_pressed_and_released()
{
  if (this->commander->temperatureManager.temperature_unit == 'C')
    this->commander->temperatureManager.temperature_unit = 'F';
  else
    this->commander->temperatureManager.temperature_unit = 'C';
  this->commander->temperatureManager.save_temp_unit();
}
