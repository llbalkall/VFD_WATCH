#include "ConcreteStates.h"

void ConcreteStateA::update_display()
{
}

void DisplayTime::update_display()
{
  
  this->commander->display_hour_minute();
  int hour = bcdToDec(this->commander->ds3231Manager.readRTCRegister(0x0C));
  int minute = bcdToDec(this->commander->ds3231Manager.readRTCRegister(0x0B));
  if (this->commander->current_time.hour == hour && this->commander->current_time.minute && this->commander->alarm_flag){
    this->commander->TransitionTo(new Alarm);
  } 
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
  this->commander->TransitionTo(new SettingNameAlarm);
}

void SettingNameAlarm::update_display()
{
  this->commander->vfdManager.update_char_array("AL Ar");
}
void SettingNameAlarm::top_pressed_and_released()
{
  this->commander->setting_value = 0 /*TODO: CURRENT_ALARM MODE*/;
  this->commander->TransitionTo(new SettingAlarmMode);
}
void SettingNameAlarm::bottom_pressed_and_released()
{
  this->commander->TransitionTo(new SettingNameTime);
}

void SettingAlarmMode::update_display()
{
  if (this->commander->setting_value == 0)
  {
    this->commander->vfdManager.update_char_array("On   ");
  }
  else if (this->commander->setting_value == 1)
  {
    this->commander->vfdManager.update_char_array("Si LE");
  }
  else if (this->commander->setting_value == 2)
  {
    this->commander->vfdManager.update_char_array("OF F ");
  }
}
void SettingAlarmMode::top_pressed_and_released()
{
  if (this->commander->setting_value == 2 /*We selected off*/)
  {
    this->commander->ds3231Manager.writeRTCRegister(0x0e, B00000100); //setting the A2IE bit to zero: disableing the alarm (other pins are good as 0)
    this->commander->TransitionTo(new DisplayTime);
  }
  else
  {
    if (this->commander->setting_value == 0)
      this->commander->alarm_sound = true;
    if (this->commander->setting_value == 1)
      this->commander->alarm_sound = false;
    this->commander->ds3231Manager.writeRTCRegister(0x0e, B00000110); //setting the A2IE bit to one: enableing the alarm
    this->commander->ds3231Manager.clearAlarmStatusBits();
    //TODO remember somehow if silent or default alarm is set
    this->commander->setting_value = bcdToDec(this->commander->ds3231Manager.readRTCRegister(0x0C));
    this->commander->TransitionTo(new SettingAlarmHour);
  }
}
void SettingAlarmMode::bottom_pressed_and_released()
{
  this->commander->setting_value += 1;
  if (this->commander->setting_value == 3)
    this->commander->setting_value = 0;
}

void SettingAlarmHour::update_display()
{
  char first_char = this->commander->setting_value / 10;
  if (first_char == 0)
    first_char = ' ';
  this->commander->vfdManager.update_char_array(first_char,
                                                this->commander->setting_value % 10,
                                                ' ',
                                                ' ',
                                                ' ');
}

void SettingAlarmHour::top_pressed_and_released()
{
  this->commander->ds3231Manager.writeRTCRegister(0x0C, decToBcd(this->commander->setting_value)); //Setting Alarm2 hour register
  this->commander->setting_value = bcdToDec(this->commander->ds3231Manager.readRTCRegister(0x0B));
  this->commander->TransitionTo(new SettingAlarmMinute);
}
void SettingAlarmHour::bottom_pressed_and_released()
{
  this->commander->setting_value += 1;
  if (this->commander->setting_value == 24)
    this->commander->setting_value = 0;
}

void SettingAlarmMinute::update_display()
{
  char first_char = this->commander->setting_value / 10;
  if (first_char == 0)
    first_char = ' ';
  this->commander->vfdManager.update_char_array(' ',
                                                ' ',
                                                ' ',
                                                first_char,
                                                this->commander->setting_value % 10);
}
void SettingAlarmMinute::top_pressed_and_released()
{
  this->commander->ds3231Manager.writeRTCRegister(0x0B, decToBcd(this->commander->setting_value)); //Setting Alarm2 minute register
  this->commander->ds3231Manager.writeRTCRegister(0x0D, B10000000);                                //Setting Alarm2 day register in a way it triggers when the hour and minute match
  this->commander->TransitionTo(new DisplayTime);
}
void SettingAlarmMinute::bottom_pressed_and_released()
{
  this->commander->setting_value += 1;
  if (this->commander->setting_value == 60)
    this->commander->setting_value = 0;
}

void SettingNameTime::update_display()
{
  this->commander->vfdManager.update_char_array("ti nn");
}

void SettingNameTime::top_pressed_and_released()
{
  this->commander->setting_value = this->commander->current_time.hour;
  this->commander->TransitionTo(new SettingHour);
}

void SettingNameTime::bottom_pressed_and_released()
{
  this->commander->TransitionTo(new SettingNameDate);
}

void SettingHour::update_display()
{
  char first_char = this->commander->setting_value / 10;
  if (first_char == 0)
    first_char = ' ';
  this->commander->vfdManager.update_char_array(first_char,
                                                this->commander->setting_value % 10,
                                                ' ',
                                                ' ',
                                                ' ');
}

void SettingHour::top_pressed_and_released()
{
  this->commander->setting_value = this->commander->current_time.minute;
  this->commander->TransitionTo(new SettingMinute);
}

void SettingHour::bottom_pressed_and_released()
{
  this->commander->setting_value += 1;
  if (this->commander->setting_value == 24)
    this->commander->setting_value = 0;
  this->commander->ds3231Manager.set_hour(this->commander->setting_value, this->commander->current_time);
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
  //this->commander->TransitionTo(new SettingNameDayOfWeek);
  this->commander->TransitionTo(new DisplayTime);
}

void SettingMinute::bottom_pressed_and_released()
{
  this->commander->setting_value += 1;
  if (this->commander->setting_value == 60)
    this->commander->setting_value = 0;
  this->commander->ds3231Manager.set_minute(this->commander->setting_value, this->commander->current_time);
}

void SettingNameDate::update_display()
{
  this->commander->vfdManager.update_char_array("DA tE");
}

void SettingNameDate::top_pressed_and_released()
{
  this->commander->setting_value = this->commander->current_time.month;
  this->commander->TransitionTo(new SettingMonth);
}

void SettingNameDate::bottom_pressed_and_released()
{
  this->commander->TransitionTo(new SettingNameDayOfWeek);
}

void SettingMonth::update_display()
{
  char first_char = this->commander->setting_value / 10;
  if (first_char == 0)
    first_char = ' ';
  this->commander->vfdManager.update_char_array(first_char,
                                                this->commander->setting_value % 10,
                                                ' ',
                                                ' ',
                                                ' ');
}

void SettingMonth::top_pressed_and_released()
{
  this->commander->setting_value = this->commander->current_time.dayOfMonth;
  if (this->commander->current_time.year % 4 == 0 &&
      this->commander->current_time.month == 2 &&
      this->commander->setting_value == 29)
  {
    //do nothing
  }
  else
  {
    if (this->commander->setting_value >= (this->commander->MONTH_LENGTHS[this->commander->current_time.month] + 1))
    {
      this->commander->setting_value = this->commander->MONTH_LENGTHS[this->commander->current_time.month];
    }
  }
  this->commander->TransitionTo(new SettingDayOfMonth);
}

void SettingMonth::bottom_pressed_and_released()
{
  this->commander->setting_value += 1;
  if (this->commander->setting_value == 13)
    this->commander->setting_value = 1;
  this->commander->ds3231Manager.set_month(this->commander->setting_value, this->commander->current_time);
}

void SettingDayOfMonth::update_display()
{
  char first_char = this->commander->setting_value / 10;
  if (first_char == 0)
    first_char = ' ';
  this->commander->vfdManager.update_char_array(' ',
                                                ' ',
                                                ' ',
                                                first_char,
                                                this->commander->setting_value % 10);
}

void SettingDayOfMonth::top_pressed_and_released()
{
  this->commander->TransitionTo(new DisplayTime);
}

void SettingDayOfMonth::bottom_pressed_and_released()
{
  this->commander->setting_value += 1;

  // Leap day condition
  if (this->commander->current_time.year % 4 == 0 &&
      this->commander->current_time.month == 2 &&
      this->commander->setting_value == 29)
  {
    //do nothing
  }
  else
  {
    if (this->commander->setting_value >= (this->commander->MONTH_LENGTHS[this->commander->current_time.month] + 1))
    {
      this->commander->setting_value = 1;
    }
  }
  this->commander->ds3231Manager.set_dayOfMonth(this->commander->setting_value, this->commander->current_time);
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
  this->commander->TransitionTo(new SettingNameYear);
}

void SettingDayOfWeek::update_display()
{
  this->commander->display_day_of_week(this->commander->setting_value);
}

void SettingDayOfWeek::top_pressed_and_released()
{
  this->commander->TransitionTo(new DisplayTime);
}

void SettingDayOfWeek::bottom_pressed_and_released()
{
  this->commander->setting_value += 1;
  if (this->commander->setting_value == 8)
    this->commander->setting_value = 1;
  this->commander->ds3231Manager.set_dayOfWeek(this->commander->setting_value, this->commander->current_time);
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
  this->commander->TransitionTo(new DisplayTime);
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
  this->commander->TransitionTo(new SettingNameAlarm);
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
  this->commander->TransitionTo(new DisplayTime);
}

void SettingTemperature::bottom_pressed_and_released()
{
  if (this->commander->temperatureManager.temperature_unit == 'C')
    this->commander->temperatureManager.temperature_unit = 'F';
  else
    this->commander->temperatureManager.temperature_unit = 'C';
  this->commander->temperatureManager.save_temp_unit();
}

void Alarm::update_display() //and this case alarming too;
{
  if (this->commander->current_millis % 1000 > 500)
  {
    this->commander->display_hour_minute();
  }
  else
  {
    this->commander->vfdManager.update_char_array("AL Ar");
  }
  this->commander->alarm_update();
}
void Alarm::top_pressed_and_released()
{
  this->commander->alarm_start_millis = this->commander->current_millis - this->commander->ALARM_DURATION;
  this->commander->alarm_counter = 0;
  //this->commander->alarm_flag = false;
  //this->commander->buzzer.turn_off();
  //this->commander->ds3231Manager.clearAlarmStatusBits();
  this->commander->turn_alarm_off();
  this->commander->TransitionTo(new DisplayTime);
}
void Alarm::bottom_pressed_and_released()
{
  this->commander->ds3231Manager.clearAlarmStatusBits();
  //buzzer_is_on = false;
  if (this->commander->alarm_counter < 4)
    this->commander->set_alarm_for_snooze();
  else
    this->commander->alarm_counter = 0;
  this->commander->alarm_flag = false;
  this->commander->buzzer.turn_off();
  this->commander->leds.turn_off();
  this->commander->TransitionTo(new SnoozeMessage);
}

void SnoozeMessage::update_display()
{
  this->commander->vfdManager.update_char_array('r',
                                                'E',
                                                ' ',
                                                '*',
                                                3 - (this->commander->alarm_counter + 3) % 4);
}
void SnoozeMessage::top_pressed_and_released()
{
  this->commander->TransitionTo(new DisplayTime);
}
void SnoozeMessage::bottom_pressed_and_released()
{
  this->commander->TransitionTo(new DisplayTime);
}

void SettingPartyModeName::update_display()
{
  this->commander->vfdManager.update_char_array("PA tY");
}

void SettingPartyModeName::top_pressed_and_released()
{
  this->commander->TransitionTo(new SettingPartyMode);
  this->commander->setting_value = 0;
  this->commander->party_mode_time_index = 0;
  this->commander->party_mode_start_time = millis();
  if (this->commander->setting_value == 0)
    this->commander->party_mode_is_on = false;
  else
    this->commander->party_mode_is_on = true;
  this->commander->party_mode_time_index = this->commander->setting_value;
}

void SettingPartyModeName::bottom_pressed_and_released()
{
  this->commander->TransitionTo(new SettingPartyMode);
  this->commander->setting_value = 0;
  this->commander->party_mode_time_index = 0;
  this->commander->party_mode_start_time = millis();
  if (this->commander->setting_value == 0)
    this->commander->party_mode_is_on = false;
  else
    this->commander->party_mode_is_on = true;
  this->commander->party_mode_time_index = this->commander->setting_value;
}

void SettingPartyMode::update_display()
{
  if (this->commander->party_mode_time_index == 9)
  {
    this->commander->vfdManager.update_char_array("99 99");
  }
  else
  {
    
    this->commander->vfdManager.update_char_array(' ',
                                                  ' ',
                                                  ' ',
                                                  this->commander->PARTY_TIMES[this->commander->setting_value] / 10 % 10,
                                                  this->commander->PARTY_TIMES[this->commander->setting_value] % 10);
  }
  if (millis() - this->commander->party_mode_start_time > 5000)
    this->commander->TransitionTo(new DisplayTime);
}
void SettingPartyMode::top_pressed_and_released()
{
  //this->commander->party_mode_time_index = this->commander->setting_value;
  this->commander->TransitionTo(new DisplayTime);
}
void SettingPartyMode::bottom_pressed_and_released()
{
  this->commander->party_mode_start_time = millis();
  this->commander->setting_value++;
  if (this->commander->setting_value > 9)
    this->commander->setting_value = 0;
  if (this->commander->setting_value == 0)
    this->commander->party_mode_is_on = false;
  else
    this->commander->party_mode_is_on = true;
  this->commander->party_mode_time_index = this->commander->setting_value;
}
