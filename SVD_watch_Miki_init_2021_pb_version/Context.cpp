#include "Context.h"

Context::Context(AbstractState *state) : state_(nullptr) {
    this->TransitionTo(state);
}

Context::~Context() {
    delete state_;
    current_time.minute= 10;
    current_millis = 1;
}

void Context::TransitionTo(AbstractState *state) {
    if (this->state_ != nullptr)
      delete this->state_;
    this->state_ = state;
    this->state_->set_context(this);
}

void Context::Update() {
    this->state_->select_control_state();
}

void Context::Request2() {
    this->state_->select_control_state();
}

void Context::display_hour_minute() {
  vfdManager.update_char_array( current_time.hour / 10, 
                                current_time.hour % 10, 
                                0, 
                                current_time.minute / 10, 
                                current_time.minute % 10);
}

void Context::display_date() {
  vfdManager.colon_steady = true;
  vfdManager.update_char_array( current_time.month / 10,       
                                current_time.month % 10,        
                                1,  
                                current_time.dayOfMonth / 10,  
                                current_time.dayOfMonth % 10);  
}

void Context::display_seconds() {//display
  vfdManager.update_char_array( ' ', 
                                ' ', 
                                ' ', 
                                current_time.second / 10, 
                                current_time.second % 10);
}

//char* days_of_week[]= {"NN on", "tu E ", "UU Ed", "th u ", "Fr i ", "SA t ", "Su n "};
void Context::display_day_of_week(byte day) {//display
  vfdManager.update_char_array(days_of_week[day-1]);
}

void Context::display_temperature() {//display and get temp
  float current_temp = temperatureManager.read_adc_to_celsius(current_millis);
  if (temperatureManager.temperature_unit == 'F') {
    current_temp = celsius_to_fahrenheit(current_temp);
    vfdManager.update_char_array(current_temp / 100,            //fh_hundreds
                                 int(current_temp / 10) % 10,   //fh_tens
                                 ' ', 
                                 int(current_temp)  % 10,       //fh_ones
                                 ' ');
  } else {
    vfdManager.colon_steady = true;
    vfdManager.update_char_array( current_temp / 10,        //celsius_tens
                                  int(current_temp) % 10,   //celsius_ones
                                  ' ', 
                                  '*', 
                                  ' ');
  }
  vfdManager.displayed_characters[4] = temperatureManager.temperature_unit;
}
