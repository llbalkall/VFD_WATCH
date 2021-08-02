#include "Commander.h"

Commander::Commander(AbstractState *state) : state_(nullptr)
{
  this->TransitionTo(state);
}

Commander::~Commander()
{
  delete state_;
  current_time.minute = 10;
  current_millis = 1;
  wake_board_millis = 0;
  stopwatch_running = false;
  alarm_start_millis = 0;
  alarm_counter = 0;
  alarm_flag = false ;
  alarm_sound = false; //save it to EEPROOM
  party_mode_time = 0;
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
  switch (buttonManager.button_state)
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
  vfdManager.update_char_array(current_time.hour / 10,
                               current_time.hour % 10,
                               0,
                               current_time.minute / 10,
                               current_time.minute % 10);
}

void Commander::display_date()
{
  vfdManager.colon_steady = true;
  vfdManager.update_char_array(current_time.month / 10,
                               current_time.month % 10,
                               1,
                               current_time.dayOfMonth / 10,
                               current_time.dayOfMonth % 10);
}

void Commander::display_seconds()
{ //display
  vfdManager.update_char_array(' ',
                               ' ',
                               ' ',
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
  if (stopwatch_running)
  {
    int elapsed_seconds = ((current_time.hour - stop_watch_time.hour) * 3600) + ((current_time.minute - stop_watch_time.minute) * 60) + (current_time.second - stop_watch_time.second);
    char elapsed_minutes = elapsed_seconds / 60;
    if (elapsed_minutes > 99)
      elapsed_minutes = 99;
    char remaining_seconds = elapsed_seconds % 60;
    vfdManager.update_char_array(elapsed_minutes / 10,
                                 elapsed_minutes % 10,
                                 1,
                                 remaining_seconds / 10,
                                 remaining_seconds % 10);
  }
  else if (stop_watch_time.second != 0 || stop_watch_time.minute != 0 || stop_watch_time.hour != 0)
  {
    vfdManager.colon_steady = true;
    int elapsed_seconds = (stop_watch_time.hour * 3600) + (stop_watch_time.minute * 60) + stop_watch_time.second;
    char elapsed_minutes = elapsed_seconds / 60;
    if (elapsed_minutes > 99)
      elapsed_minutes = 99;
    char remaining_seconds = elapsed_seconds % 60;
    vfdManager.update_char_array(elapsed_minutes / 10,
                                 elapsed_minutes % 10,
                                 1,
                                 remaining_seconds / 10,
                                 remaining_seconds % 10);
  }
  else
  {
    vfdManager.colon_steady = true;
    vfdManager.update_char_array(0, 0, 1, 0, 0);
  }
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
  digitalWrite(LED_1_PIN, HIGH);
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

void Commander::alarm(){
  long t = current_millis - alarm_start_millis;
  if (alarm_flag){
    //if (alarm_sound) buzz_for_alarm();
    if (t> ALARM_DURATION) {
      alarm_flag = false;
      ds3231Manager.clearAlarmStatusBits();
      //buzzer_is_on=false;
      if (alarm_counter>2){
      }
    }
  } 
}
