#include "Commander.h"

Commander::Commander(AbstractState *state) : state_(nullptr)
{
  this->TransitionTo(state);
}

Commander::~Commander()
{
  delete state_;
  current_millis = 1;
  wake_board_millis = 0;

  stopwatch_running = false;

  alarm_start_millis = 0;
  alarm_counter = 0;
  alarm_flag = false ;
  alarm_sound = false; //save it to EEPROOM

  waking_up = false;
  first_wake_up = true;

  is_second_setting = false;

  party_mode_is_on = false;
  party_mode_start_time = 0;
  party_mode_time_index = 0;

  back_to_the_future_animation_state = 0;
  bttf_animation_start_millis = 0;
}

void Commander::TransitionTo(AbstractState *state)
{
  if (this->state_ != nullptr)
    delete this->state_;
  this->state_ = state;
  this->state_->set_context(this);
}

void Commander::Update()
{
  this->state_->update_display();
  // States: 0 - nothing pressed, 1 - #1 pressed and released, 2 - #2 pressed and released
  // 3 - #1 held, 4 - #2 held, 5 - both held
  if (waking_up) {
    if (first_wake_up) {
      buttonManager.first_wake_up_init();
      first_wake_up = false;
    }
    else  buttonManager.wake_up_init();
    waking_up = false;
  }
  /*if (buttonManager.state > 0){
    buzzer.set_last_input_millis(current_millis);
  }
  if (current_millis - buzzer.get_last_input_millis() < 10) {
    buzzer.turn_on();
  } else if (current_millis - buzzer.get_last_input_millis() > 50 && 
            current_millis - buzzer.get_last_input_millis() < 60){
    buzzer.turn_off();
  }*/
  switch (buttonManager.state)
  {
  case 1:
    this->state_->top_pressed_and_released();
    break;
  case 2:
    this->state_->bottom_pressed_and_released();
    break;
  case 3:
    this->state_->top_held();
    break;
  case 4:
    this->state_->bottom_held();
    break;
  case 5:
    this->state_->both_held();
    break;
  default:
    break;
  }
}

void Commander::display_hour_minute()
{
  char first_hour_char = current_time.hour / 10;
  if (first_hour_char == 0) first_hour_char = ' ';
  vfdManager.update_char_array(first_hour_char,
                               current_time.hour % 10,
                               0,
                               current_time.minute / 10,
                               current_time.minute % 10);
}

void Commander::display_date()
{
  //vfdManager.colon_steady = true;
  char month_first_digit = current_time.month / 10;
  if (month_first_digit == 0) month_first_digit = ' ';   
  char day_first_digit = current_time.dayOfMonth / 10;
  if (day_first_digit == 0) day_first_digit = ' ';   
  vfdManager.update_char_array(month_first_digit,
                               current_time.month % 10,
                               ' ',
                               day_first_digit,
                               current_time.dayOfMonth % 10);
}

void Commander::display_seconds()
{ //display
  vfdManager.colon_steady = true;
  vfdManager.update_char_array(' ',
                               ' ',
                               1,
                               current_time.second / 10,
                               current_time.second % 10);
}

//char* days_of_week[]= {"NN on", "tu E ", "UU Ed", "th u ", "Fr i ", "SA t ", "Su n "};
void Commander::display_day_of_week(byte day)
{ //display
  vfdManager.update_char_array(days_of_week[day - 1]);
}

void Commander::display_temperature()
{ //display and get temp
  float current_temp = temperatureManager.read_adc_to_celsius(current_millis);
  if (temperatureManager.temperature_unit == 'F')
  {
    current_temp = celsius_to_fahrenheit(current_temp);
    vfdManager.update_char_array(current_temp / 100,          //fh_hundreds
                                 int(current_temp / 10) % 10, //fh_tens
                                 ' ',
                                 int(current_temp) % 10, //fh_ones
                                 ' ');
  }
  else
  {
    vfdManager.colon_steady = true;
    vfdManager.update_char_array(current_temp / 10,      //celsius_tens
                                 int(current_temp) % 10, //celsius_ones
                                 ' ',
                                 '*',
                                 ' ');
  }
  vfdManager.displayed_characters[4] = temperatureManager.temperature_unit;
}

void Commander::display_stopwatch()
{ 
  unsigned long elapsed_seconds = stopper.get_elapsed_sec();
  char elapsed_minutes = elapsed_seconds / 60;
  if (elapsed_minutes > 99 && !stopper.overflowed){
    stopper.overflowed = true;
  }

  if (stopper.overflowed) {
    elapsed_minutes = 99;
    elapsed_seconds = 59;
  }
   
  char remaining_seconds = elapsed_seconds % 60;
  vfdManager.update_char_array(elapsed_minutes / 10,
                                elapsed_minutes % 10,
                                1,
                                remaining_seconds / 10,
                                remaining_seconds % 10);
}

void Commander::read_current_time()
{ 
  ds3231Manager.readDS3231time( &current_time.second, &current_time.minute, &current_time.hour, &current_time.dayOfWeek,
                                &current_time.dayOfMonth, &current_time.month, &current_time.year);
}

void Commander::flash_leds()
{ //output
  int led_millis = current_millis - wake_board_millis;
  if (led_millis < LOW_BATTERY_MESSAGE_DISPLAY_DURATION && led_millis % (2 * LED_FLASH_INTERVAL) < LED_FLASH_INTERVAL)
  {
    leds.turn_on();
  }
  else if (led_millis < LOW_BATTERY_MESSAGE_DISPLAY_DURATION)
  {
    leds.turn_off();
  }
}

void Commander::set_alarm_for_snooze(){
  int snooze_length = 1;
  int snooze_hour = 0;
  int snooze_minute = 0;
  snooze_minute += current_time.minute + snooze_length;
  snooze_hour = current_time.hour;
  if (snooze_minute>59) {
    snooze_minute -= 60;
    snooze_hour = current_time.hour + 1;
    if (snooze_hour>23){
      snooze_hour -= 24;
    }
  } 
  ds3231Manager.writeRTCRegister(0x07, B00000001) ; //ALARM1 seconds reg
  ds3231Manager.writeRTCRegister(0x09, decToBcd(snooze_hour)) ; //Setting Alarm1 hour register
  ds3231Manager.writeRTCRegister(0x08, decToBcd(snooze_minute)) ; //Setting Alarm1 minute register
  ds3231Manager.writeRTCRegister(0x0A, B10000000); //Setting Alarm1 day register in a way it triggers when the hour and minute match and second but it's set to 0
  ds3231Manager.writeRTCRegister(0x0e, B00000101);//enable alarm1, disable alarm2
}

void Commander::alarm_update(){
  long t = current_millis - alarm_start_millis;
  if (alarm_flag){
    if (alarm_sound) buzzer.alarm(current_millis);
    leds.alarm(current_millis);
    //if (alarm_sound) buzz_for_alarm();
    if (t> ALARM_DURATION) {
      turn_alarm_off();
    }
  }  
}

void Commander::trigger_alarm(){
  if (!alarm_flag){
    alarm_start_millis = current_millis;
    alarm_flag = true;
    alarm_counter++;
    //TransitionTo( new Alarm);
  }
}

void Commander::turn_alarm_off(){
  ds3231Manager.clearAlarmStatusBits();
  alarm_flag = false;
  buzzer.turn_off();
  leds.turn_off();
}
