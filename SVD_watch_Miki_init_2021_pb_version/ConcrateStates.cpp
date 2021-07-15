#include "ConcreteStates.h"


void ConcreteStateA::select_control_state(){

}

void DisplayTime::select_control_state(){
  this->context_->display_hour_minute();
  if (this->context_->buttonManager.button_state == 1) this->context_->TransitionTo(new DisplayDate);
  else if (this->context_->buttonManager.button_state == 2) this->context_->TransitionTo(new DisplayDayOfWeek);
}

void DisplayDate::select_control_state(){
  this->context_->display_date();
  if (this->context_->buttonManager.button_state == 1) this->context_->TransitionTo(new DisplaySeconds);
  else if (this->context_->buttonManager.button_state == 2) this->context_->TransitionTo(new DisplayDayOfWeek);
}

void DisplaySeconds::select_control_state(){
  this->context_->display_seconds();
  if (this->context_->buttonManager.button_state == 1) this->context_->TransitionTo(new DisplayTime);
  else if (this->context_->buttonManager.button_state == 2) this->context_->TransitionTo(new DisplayDayOfWeek);
}

void DisplayDayOfWeek::select_control_state(){
  this->context_->display_day_of_week(this->context_->current_time.dayOfWeek);
  if (this->context_->buttonManager.button_state == 1) this->context_->TransitionTo(new DisplayTime);
  else if (this->context_->buttonManager.button_state == 2) this->context_->TransitionTo(new DisplayTemperature);
}

void DisplayTemperature::select_control_state(){
  this->context_->display_temperature();
  if (this->context_->buttonManager.button_state == 1) this->context_->TransitionTo(new DisplayTime);
  else if (this->context_->buttonManager.button_state == 2) this->context_->TransitionTo(new DisplayTime);
}
