#include "ConcreteStates.h"


void ConcreteStateA::update_display(){

}

void DisplayTime::update_display(){
  this->context_->display_hour_minute();
}

void DisplayTime::first_pressed_and_released(){
  this->context_->TransitionTo(new DisplayDate);
}

void DisplayTime::second_pressed_and_released() {
  this->context_->TransitionTo(new DisplayDayOfWeek);
}


void DisplayDate::update_display(){
  this->context_->display_date();
}

void DisplayDate::first_pressed_and_released(){
  this->context_->TransitionTo(new DisplaySeconds);
}

void DisplayDate::second_pressed_and_released() {
  this->context_->TransitionTo(new DisplayDayOfWeek);
}


void DisplaySeconds::update_display(){
  this->context_->display_seconds();
}

void DisplaySeconds::first_pressed_and_released(){
  this->context_->TransitionTo(new DisplayTime);
}

void DisplaySeconds::second_pressed_and_released() {
  this->context_->TransitionTo(new DisplayDayOfWeek);
}


void DisplayDayOfWeek::update_display(){
  this->context_->display_day_of_week(this->context_->current_time.dayOfWeek);
}

void DisplayDayOfWeek::first_pressed_and_released(){
  this->context_->TransitionTo(new DisplayTime);
}

void DisplayDayOfWeek::second_pressed_and_released() {
  this->context_->TransitionTo(new DisplayTemperature);
}


void DisplayTemperature::update_display(){
  this->context_->display_temperature();
}

void DisplayTemperature::first_pressed_and_released(){
  this->context_->TransitionTo(new DisplayTime);
}

void DisplayTemperature::second_pressed_and_released() {
  this->context_->TransitionTo(new DisplayTime);
}



void StopWatch::update_display(){
  this->context_->display_stopwatch();
}

void StopWatch::first_pressed_and_released(){
  this->context_->TransitionTo(new DisplayTime);
}

void StopWatch::second_pressed_and_released() {
  if (this->context_->stopwatch_running) {
    this->context_->stopwatch_running = false;
    this->context_->stop_watch_time.dayOfMonth = this->context_->current_time.dayOfMonth - this->context_->stop_watch_time.dayOfMonth;
    this->context_->stop_watch_time.hour = this->context_->current_time.hour - this->context_->stop_watch_time.hour;
    this->context_->stop_watch_time.minute = this->context_->current_time.minute - this->context_->stop_watch_time.minute;
    this->context_->stop_watch_time.second = this->context_->current_time.second - this->context_->stop_watch_time.second;
  }
  else if (this->context_->stop_watch_time.second == 0 && this->context_->stop_watch_time.minute == 0 && this->context_->stop_watch_time.hour == 0) {
    this->context_->stopwatch_running = true;
    this->context_->stop_watch_time.second = this->context_->current_time.second;
    this->context_->stop_watch_time.minute = this->context_->current_time.minute;
    this->context_->stop_watch_time.hour = this->context_->current_time.hour;
    this->context_->stop_watch_time.dayOfMonth = this->context_->current_time.dayOfMonth;
  } else {
    this->context_->stop_watch_time.second = 0;
    this->context_->stop_watch_time.minute = 0;
    this->context_->stop_watch_time.hour = 0;
    this->context_->stop_watch_time.dayOfMonth = 0;
  }
}


void EnterSettings::update_display(){
  this->context_->vfdManager.update_char_array("SE t ");
}

void EnterSettings::first_pressed_and_released(){
  this->context_->TransitionTo(new DisplayTime);
}

void EnterSettings::second_pressed_and_released() {
  this->context_->TransitionTo(new SettingNameHour);
}


void SettingNameHour::update_display(){
  this->context_->vfdManager.update_char_array("Ho ur");
}

void SettingNameHour::first_pressed_and_released(){
  this->context_->setting_value = this->context_->current_time.hour;
  this->context_->TransitionTo(new SettingHour);
}

void SettingNameHour::second_pressed_and_released() {
  this->context_->TransitionTo(new SettingNameMinute);
}


void SettingHour::update_display(){
  this->context_->vfdManager.update_char_array( this->context_->setting_value / 10, 
                                this->context_->setting_value % 10, 
                                ' ', 
                                ' ', 
                                ' ');
}

void SettingHour::first_pressed_and_released(){
  this->context_->TransitionTo(new SettingNameMinute);
}

void SettingHour::second_pressed_and_released() {
  this->context_->setting_value += 1;
  if (this->context_->setting_value == 24) this->context_->setting_value = 0;
  this->context_->ds3231Manager.set_hour(this->context_->setting_value, this->context_->current_time);
}


void SettingNameMinute::update_display(){
  this->context_->vfdManager.update_char_array("NN in");
}

void SettingNameMinute::first_pressed_and_released(){
  this->context_->TransitionTo(new SettingMinute);
  this->context_->setting_value = this->context_->current_time.minute;
}

void SettingNameMinute::second_pressed_and_released() {
  this->context_->vfdManager.displayed_characters[3] = 'o';
  this->context_->TransitionTo(new SettingNameDayOfWeek);
}


void SettingMinute::update_display(){
  this->context_->vfdManager.update_char_array( ' ', 
                                                ' ', 
                                                ' ', 
                                                this->context_->setting_value / 10, 
                                                this->context_->setting_value % 10);
}

void SettingMinute::first_pressed_and_released(){
  this->context_->TransitionTo(new SettingNameDayOfWeek);
}

void SettingMinute::second_pressed_and_released() {
  this->context_->setting_value += 1;
  if (this->context_->setting_value == 60) this->context_->setting_value = 0;
  this->context_->ds3231Manager.set_minute(this->context_->setting_value, this->context_->current_time);
}


void SettingNameDayOfWeek::update_display(){
  this->context_->vfdManager.update_char_array("do UU");
}

void SettingNameDayOfWeek::first_pressed_and_released(){
  this->context_->setting_value = this->context_->current_time.dayOfWeek;
  this->context_->TransitionTo(new SettingDayOfWeek);
}

void SettingNameDayOfWeek::second_pressed_and_released() {
  this->context_->TransitionTo(new SettingNameDayOfMonth);
}
  
 
void SettingDayOfWeek::update_display(){
  this->context_->display_day_of_week(this->context_->setting_value);
}

void SettingDayOfWeek::first_pressed_and_released(){
  this->context_->TransitionTo(new SettingNameDayOfMonth);
}

void SettingDayOfWeek::second_pressed_and_released() {
  this->context_->setting_value += 1;
  if (this->context_->setting_value == 8) this->context_->setting_value = 1;
  this->context_-> ds3231Manager.set_dayOfWeek(this->context_->setting_value, this->context_->current_time);
}


void SettingNameDayOfMonth::update_display(){
  this->context_->vfdManager.update_char_array("do NN");
}

void SettingNameDayOfMonth::first_pressed_and_released(){
  this->context_->setting_value = this->context_->current_time.dayOfMonth;
  this->context_->TransitionTo(new SettingDayOfMonth);
}

void SettingNameDayOfMonth::second_pressed_and_released() {
  this->context_->TransitionTo(new SettingNameMonth);
}
  

void SettingDayOfMonth::update_display(){
  this->context_->vfdManager.update_char_array( ' ', 
                                                ' ', 
                                                ' ', 
                                                this->context_->setting_value / 10, 
                                                this->context_->setting_value % 10);
}

void SettingDayOfMonth::first_pressed_and_released(){
  this->context_->TransitionTo(new SettingNameMonth);
}

void SettingDayOfMonth::second_pressed_and_released() {
  this->context_->setting_value += 1;
  if (this->context_->setting_value == (this->context_->MONTH_LENGTHS[this->context_->current_time.month] + 1)) {
    this->context_->setting_value = 1;
  }
  // Leap day condition
  if (this->context_->current_time.year % 4 == 0 && 
      this->context_->current_time.month == 2 && 
      this->context_->setting_value == 29){
    this->context_->setting_value = 1;
  }
  this->context_->ds3231Manager.set_dayOfMonth(this->context_->setting_value, this->context_->current_time);
}


void SettingNameMonth::update_display(){
  this->context_->vfdManager.update_char_array("NN on");
}

void SettingNameMonth::first_pressed_and_released(){
  this->context_->setting_value = this->context_->current_time.month;
  this->context_->TransitionTo(new SettingMonth);
}

void SettingNameMonth::second_pressed_and_released() {
  this->context_->TransitionTo(new SettingNameYear);
}
 

void SettingMonth::update_display(){
  this->context_->vfdManager.update_char_array( this->context_->setting_value / 10, 
                                                this->context_->setting_value % 10, 
                                                ' ', 
                                                ' ', 
                                                ' ');
}

void SettingMonth::first_pressed_and_released(){
  this->context_->TransitionTo(new SettingNameYear);
}

void SettingMonth::second_pressed_and_released() {
  this->context_->setting_value += 1;
  if (this->context_->setting_value == 13) this->context_->setting_value = 1;
  this->context_->ds3231Manager.set_month( this->context_->setting_value,  this->context_->current_time);
}


void SettingNameYear::update_display(){
  this->context_->vfdManager.update_char_array("YE Ar");
}

void SettingNameYear::first_pressed_and_released(){
  this->context_->setting_value =  this->context_->current_time.year;
  this->context_->TransitionTo(new SettingYear);
}

void SettingNameYear::second_pressed_and_released() {
  this->context_->TransitionTo(new SettingNameTemperature);
}


void SettingYear::update_display(){
  this->context_->vfdManager.update_char_array( '2', 
                                                '0', 
                                                ' ', 
                                                this->context_->setting_value / 10, 
                                                this->context_->setting_value % 10);
}

void SettingYear::first_pressed_and_released(){
 this->context_->TransitionTo(new SettingNameTemperature);
}

void SettingYear::second_pressed_and_released() {
  this->context_->setting_value += 1;
  if (this->context_->setting_value == 100) this->context_->setting_value = 20;
  this->context_->ds3231Manager.set_year(this->context_->setting_value, this->context_->current_time);
}


void SettingNameTemperature::update_display(){
  this->context_->vfdManager.update_char_array("tE nn");
}

void SettingNameTemperature::first_pressed_and_released(){
  this->context_->TransitionTo(new SettingTemperature);
}

void SettingNameTemperature::second_pressed_and_released() {
  this->context_->TransitionTo(new SettingNameHour);
}


void SettingTemperature::update_display(){
  this->context_->vfdManager.update_char_array( ' ', 
                                                ' ', 
                                                ' ', 
                                                '*', 
                                                this->context_->temperatureManager.temperature_unit);
}

void SettingTemperature::first_pressed_and_released(){
  this->context_->TransitionTo(new SettingNameHour);
}

void SettingTemperature::second_pressed_and_released() {
  if (this->context_->temperatureManager.temperature_unit == 'C') this->context_->temperatureManager.temperature_unit = 'F';
  else this->context_->temperatureManager.temperature_unit = 'C'; 
  this->context_->temperatureManager.save_temp_unit();
}
