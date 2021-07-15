#include <Wire.h>
#include <avr/sleep.h>
#include <avr/interrupt.h> 
#include "functions.h"
#include <VFDManager.h>
#include <DS3231Manager.h>
#include <SimpleTime.h>
#include <BatteryReadingManager.h>
#include "Context.h"
#include "ConcreteStates.h"

#define POWERSENSE_PIN A2
#define BUZZER_PIN 3
#define POWER_MEASURE_PIN A7

// Clarified listing of control states the watch can be in
const uint16_t DISPLAY_TIME = 0;
const uint16_t DISPLAY_DATE = 1;
const uint16_t DISPLAY_SECONDS = 2;
const uint16_t DISPLAY_DAY_OF_WEEK = 3;
const uint16_t DISPLAY_TEMPERATURE = 4;
const uint16_t ENTER_SETTINGS = 5;
const uint16_t SETTING_NAME_HOUR = 6;
const uint16_t SETTING_HOUR = 7;
const uint16_t SETTING_NAME_MINUTE = 8;
const uint16_t SETTING_MINUTE = 9;
const uint16_t SETTING_NAME_DAY_OF_WEEK = 10;
const uint16_t SETTING_DAY_OF_WEEK = 11;
const uint16_t SETTING_NAME_DAY_OF_MONTH = 12;
const uint16_t SETTING_DAY_OF_MONTH = 13;
const uint16_t SETTING_NAME_MONTH = 14;
const uint16_t SETTING_MONTH = 15;
const uint16_t SETTING_NAME_YEAR = 16;
const uint16_t SETTING_YEAR = 17;
const uint16_t SETTING_NAME_TEMPERATURE = 18;
const uint16_t SETTING_TEMPERATURE = 19;
const uint16_t STOPWATCH = 20;

const uint16_t SLEEP_TIMEOUT_INTERVAL = 25000;
const uint16_t LOW_BATTERY_MESSAGE_DISPLAY_DURATION = 2000;
const uint16_t LED_FLASH_INTERVAL = 150;

void run_control_state();
void read_current_time();
void power_board_down(bool permit_wakeup);
void display_stopwatch();
void flash_leds();

volatile uint16_t control_state = 0;

volatile bool board_sleeping = false;
bool stopwatch_running = false;
byte adcsra, mcucr1, mcucr2;
uint16_t setting_value = 0;

volatile unsigned long last_input_millis = 0;
volatile unsigned long wake_board_millis = 0;
unsigned long current_millis = 0;
volatile unsigned long last_battery_read_millis = 0;
const uint16_t MONTH_LENGTHS[13] = {0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

DS3231Manager ds3231Manager;
//VFDManager vfdManager = VFDManager();
BatteryReadingManager batteryReadingManager;
LEDs leds;
//Time current_time;
Time stop_watch_time;
//ButtonManager buttonManager_unused;
//TemperatureManager temperatureManager;
Context *context = new Context(new DisplayTime);


/*ButtonManager buttonManager;
  VFDManager vfdManager = VFDManager();
  Time current_time;
  TemperatureManager temperatureManager;
  unsigned long current_millis;*/


void setup() { //input, output init, setting up interrupts, timers
  pinMode(POWER_MEASURE_PIN, OUTPUT);
  digitalWrite(POWER_MEASURE_PIN, LOW);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  power_board_down(true);
}

void select_control_state();

void loop() {
  current_millis = millis();
  context->current_millis = current_millis;
  if (batteryReadingManager.battery_level == batteryReadingManager.BATTERY_READING){
     batteryReadingManager.read_battery_level(current_millis);
  } else if (batteryReadingManager.battery_level == batteryReadingManager.INSTANT_TURN_OFF) {
    power_board_down(false);
  } else {
    if (!board_sleeping && context->vfdManager.repower) {
      //digitalWrite(VFD_POWER_SWITCH_PIN, HIGH);
      context->vfdManager.turn_on();
      if (batteryReadingManager.battery_level != batteryReadingManager.TOO_LOW_FOR_DISPLAY){
        digitalWrite(LOAD_PIN, HIGH);//TODO, what is this? VFD-s load pin
      } 
      context->vfdManager.repower = false;
      wake_board_millis = current_millis;
      last_input_millis = current_millis;
    }
    // States: 0 - nothing pressed, 1 - #1 pressed and released, 2 - #2 pressed and released
    // 3 - #1 held, 4 - #2 held, 5 - both held
    if (batteryReadingManager.battery_level != batteryReadingManager.TOO_LOW_FOR_DISPLAY) {
      context->buttonManager.update_button_state();
      read_current_time();
      
      //context->current_time.setTime(current_time);
      context->Update();
      //select_control_state();
      //vfdManager.debug_4_digit(8888);
      //delay(3);
      context->vfdManager.show_displayed_character_array(current_millis);
    }
    if (batteryReadingManager.battery_level == batteryReadingManager.GETTING_LOW || 
        batteryReadingManager.battery_level == batteryReadingManager.TOO_LOW_FOR_DISPLAY) {
      flash_leds();
    }
    // End of interactive loop; all necessary input from button has been registered, reset it
    if (context->buttonManager.button_state != 0) last_input_millis = current_millis;
    context->buttonManager.button_state = 0; 
    context->vfdManager.colon_steady = false;
    if (current_millis - last_input_millis > SLEEP_TIMEOUT_INTERVAL && !board_sleeping) power_board_down(true);
  }
}

void read_current_time() { //DS32131
  ds3231Manager.readDS3231time(&context->current_time.second, &context->current_time.minute, &context->current_time.hour, &context->current_time.dayOfWeek,
  &context->current_time.dayOfMonth, &context->current_time.month, &context->current_time.year);
}
/*
void select_control_state() {
  // Launch relevant state functions
  // Switch states dependent on button state
  switch(control_state) {
    case DISPLAY_TIME:
      display_hour_minute();
      if (buttonManager_unused.button_state == 1) control_state = DISPLAY_DATE;
      else if (buttonManager_unused.button_state == 2) control_state = DISPLAY_DAY_OF_WEEK;
      break;
    case DISPLAY_DATE:
      display_date();
      if (buttonManager_unused.button_state == 1) control_state = DISPLAY_SECONDS;
      else if (buttonManager_unused.button_state == 2) control_state = DISPLAY_DAY_OF_WEEK;
      break;
    case DISPLAY_SECONDS:
      display_seconds();
      if (buttonManager_unused.button_state == 1) control_state = DISPLAY_TIME;
      else if (buttonManager_unused.button_state == 2) control_state = DISPLAY_DAY_OF_WEEK;
    break;
    case DISPLAY_DAY_OF_WEEK:
      display_day_of_week(context->current_time.dayOfWeek);
      if (buttonManager_unused.button_state == 1) control_state = DISPLAY_TIME;
      else if (buttonManager_unused.button_state == 2) control_state = DISPLAY_TEMPERATURE;
      break;
    case DISPLAY_TEMPERATURE:
      display_temperature();
      if (buttonManager_unused.button_state == 1) control_state = DISPLAY_TIME;
      else if (buttonManager_unused.button_state == 2) control_state = DISPLAY_TIME;
      break;
    case STOPWATCH:
      display_stopwatch();
      if (buttonManager_unused.button_state == 1) control_state = DISPLAY_TIME;
      else if (buttonManager_unused.button_state == 2) {
        if (stopwatch_running) {
          stopwatch_running = false;
          stop_watch_time.dayOfMonth = context->current_time.dayOfMonth - stop_watch_time.dayOfMonth;
          stop_watch_time.hour = context->current_time.hour - stop_watch_time.hour;
          stop_watch_time.minute = context->current_time.minute - stop_watch_time.minute;
          stop_watch_time.second = context->current_time.second - stop_watch_time.second;
        }
        else if (stop_watch_time.second == 0 && stop_watch_time.minute == 0 && stop_watch_time.hour == 0) {
          stopwatch_running = true;
          stop_watch_time.second = context->current_time.second;
          stop_watch_time.minute = context->current_time.minute;
          stop_watch_time.hour = context->current_time.hour;
          stop_watch_time.dayOfMonth = context->current_time.dayOfMonth;
        } else {
          stop_watch_time.second = 0;
          stop_watch_time.minute = 0;
          stop_watch_time.hour = 0;
          stop_watch_time.dayOfMonth = 0;
        }
      }
      break;
    case ENTER_SETTINGS:
      vfdManager.update_char_array("SE t ");
      if (buttonManager_unused.button_state == 1) control_state = DISPLAY_TIME;
      else if (buttonManager_unused.button_state == 2) control_state = SETTING_NAME_HOUR;
      break;
    case SETTING_NAME_HOUR:
      vfdManager.update_char_array("Ho ur");
      if (buttonManager_unused.button_state == 1) { 
        control_state = SETTING_HOUR;
        setting_value = context->current_time.hour;
      } else if (buttonManager_unused.button_state == 2) control_state = SETTING_NAME_MINUTE;
      break;
    case SETTING_HOUR:
      vfdManager.update_char_array(setting_value / 10, setting_value % 10, ' ', ' ', ' ');
      if (buttonManager_unused.button_state == 2) { 
        setting_value += 1;
        if (setting_value == 24) setting_value = 0;
      }
      else if (buttonManager_unused.button_state == 1) {
        ds3231Manager.set_hour(setting_value, context->current_time);
        control_state = SETTING_NAME_MINUTE;
      }
      break;
    case SETTING_NAME_MINUTE:
      vfdManager.update_char_array("NN in");
      if (buttonManager_unused.button_state == 1) { 
        control_state = SETTING_MINUTE;
        setting_value = context->current_time.minute;
      }
      else if (buttonManager_unused.button_state == 2) {
        control_state = SETTING_NAME_DAY_OF_WEEK;
        vfdManager.displayed_characters[3] = 'o';
      }
      break;
    case SETTING_MINUTE:
      vfdManager.update_char_array(' ', ' ', ' ', setting_value / 10, setting_value % 10);
      if (buttonManager_unused.button_state == 2) { 
        setting_value += 1;
        if (setting_value == 60) setting_value = 0;
      }
      else if (buttonManager_unused.button_state == 1) {
        ds3231Manager.set_minute(setting_value, context->current_time);
        control_state = SETTING_NAME_DAY_OF_WEEK;
      }
      break;
    case SETTING_NAME_DAY_OF_WEEK:
      vfdManager.update_char_array("do UU");
      if (buttonManager_unused.button_state == 1) { 
        control_state = SETTING_DAY_OF_WEEK;
        setting_value = context->current_time.dayOfWeek;
      }
      else if (buttonManager_unused.button_state == 2) control_state = SETTING_NAME_DAY_OF_MONTH;
      break;
    case SETTING_DAY_OF_WEEK:
    display_day_of_week(setting_value);
      if (buttonManager_unused.button_state == 2) { 
        setting_value += 1;
        if (setting_value == 8) setting_value = 1;
      }
      else if (buttonManager_unused.button_state == 1) {
        ds3231Manager.set_dayOfWeek(setting_value, context->current_time);
        control_state = SETTING_NAME_DAY_OF_MONTH;
      }
      break;
    case SETTING_NAME_DAY_OF_MONTH:
      vfdManager.update_char_array("do NN");
      if (buttonManager_unused.button_state == 1) { 
        control_state = SETTING_DAY_OF_MONTH;
        setting_value = context->current_time.dayOfMonth;
      }
      else if (buttonManager_unused.button_state == 2) control_state = SETTING_NAME_MONTH;
      break;
    case SETTING_DAY_OF_MONTH:
      vfdManager.update_char_array(' ', ' ', ' ', setting_value / 10, setting_value % 10);
      if (buttonManager_unused.button_state == 2) { 
        setting_value += 1;
        if (setting_value == (MONTH_LENGTHS[context->current_time.month] + 1)) setting_value = 1;
        // Leap day condition
        if (context->current_time.year % 4 == 0 && context->current_time.month == 2 && setting_value == 29) setting_value = 1;
      }
      else if (buttonManager_unused.button_state == 1) {
        ds3231Manager.set_dayOfMonth(setting_value, context->current_time);
        control_state = SETTING_NAME_MONTH;
      }
      break;
    case SETTING_NAME_MONTH:
      vfdManager.update_char_array("NN on");
      if (buttonManager_unused.button_state == 1) { 
        control_state = SETTING_MONTH;
        setting_value = context->current_time.month;
      }
      else if (buttonManager_unused.button_state == 2) control_state = SETTING_NAME_YEAR;
      break;
    case SETTING_MONTH:
      vfdManager.update_char_array(setting_value / 10, setting_value % 10, ' ', ' ', ' ');
      if (buttonManager_unused.button_state == 2) { 
        setting_value += 1;
        if (setting_value == 13) setting_value = 1;
      }
      else if (buttonManager_unused.button_state == 1) {
        ds3231Manager.set_month(setting_value, context->current_time);
        control_state = SETTING_NAME_YEAR;
      }
      break;
    case SETTING_NAME_YEAR:
      vfdManager.update_char_array("YE Ar");
      if (buttonManager_unused.button_state == 1) { 
        control_state = SETTING_YEAR;
        setting_value = context->current_time.year;
      }
      else if (buttonManager_unused.button_state == 2) control_state = SETTING_NAME_TEMPERATURE;
      break;
    case SETTING_YEAR:
      vfdManager.update_char_array('2', '0', ' ', setting_value / 10, setting_value % 10);
      if (buttonManager_unused.button_state == 2) { 
        setting_value += 1;
        if (setting_value == 100) setting_value = 20;
      }
      else if (buttonManager_unused.button_state == 1) {
        ds3231Manager.set_year(setting_value, context->current_time);
        control_state = SETTING_NAME_TEMPERATURE;
      }
      break;
    case SETTING_NAME_TEMPERATURE:
      vfdManager.update_char_array("tE nn");
      if (buttonManager_unused.button_state == 1) control_state = SETTING_TEMPERATURE;
      else if (buttonManager_unused.button_state == 2) control_state = SETTING_NAME_HOUR;
      break;
    case SETTING_TEMPERATURE:
      vfdManager.update_char_array(' ', ' ', ' ', '*', context->temperatureManager.temperature_unit);
      if (buttonManager_unused.button_state == 1) {
        control_state = SETTING_NAME_HOUR;
        context->temperatureManager.save_temp_unit();
        //EEPROM.write(temperature_unit_eeprom_address, temperature_unit);
      }
      else if (buttonManager_unused.button_state == 2) {
        if (context->temperatureManager.temperature_unit == 'C') context->temperatureManager.temperature_unit = 'F';
        else context->temperatureManager.temperature_unit = 'C'; 
      }
      break;
  }
  if (buttonManager_unused.button_state == 4) control_state = ENTER_SETTINGS;
  else if (buttonManager_unused.button_state == 3) control_state = STOPWATCH;
  if (batteryReadingManager.battery_level == batteryReadingManager.GETTING_LOW && current_millis - wake_board_millis < LOW_BATTERY_MESSAGE_DISPLAY_DURATION) {
    vfdManager.update_char_array("BA Lo");
  }  
}
*/
void display_stopwatch() {
  if (stopwatch_running) {
    int elapsed_seconds = ((context->current_time.hour - stop_watch_time.hour) * 3600) + ((context->current_time.minute - stop_watch_time.minute) * 60) + (context->current_time.second - stop_watch_time.second);
    char elapsed_minutes = elapsed_seconds / 60;
    if (elapsed_minutes > 99) elapsed_minutes = 99;
    char remaining_seconds = elapsed_seconds % 60;
    context->vfdManager.update_char_array( elapsed_minutes / 10, 
                                  elapsed_minutes % 10, 
                                  1, 
                                  remaining_seconds / 10, 
                                  remaining_seconds % 10);
  } else if (stop_watch_time.second != 0 || stop_watch_time.minute != 0 || stop_watch_time.hour != 0){
    context->vfdManager.colon_steady = true;
    int elapsed_seconds = (stop_watch_time.hour * 3600) + (stop_watch_time.minute * 60) + stop_watch_time.second;
    char elapsed_minutes = elapsed_seconds / 60;
    if (elapsed_minutes > 99) elapsed_minutes = 99;
    char remaining_seconds = elapsed_seconds % 60;
    context->vfdManager.update_char_array( elapsed_minutes / 10,
                                  elapsed_minutes % 10,
                                  1,
                                  remaining_seconds / 10,
                                  remaining_seconds % 10);
  } else {
    context->vfdManager.colon_steady = true;
    context->vfdManager.update_char_array(0, 0, 1, 0, 0);
  
  }
}
/*
void display_hour_minute() {
  vfdManager.update_char_array( context->current_time.hour / 10, 
                                context->current_time.hour % 10, 
                                1, 
                                context->current_time.minute / 10, 
                                context->current_time.minute % 10);
}

void display_date() {
  vfdManager.colon_steady = true;
  vfdManager.update_char_array( context->current_time.month / 10,       
                                context->current_time.month % 10,        
                                1,  
                                context->current_time.dayOfMonth / 10,  
                                context->current_time.dayOfMonth % 10);  
}

void display_seconds() {//display
  vfdManager.update_char_array( ' ', 
                                ' ', 
                                ' ', 
                                context->current_time.second / 10, 
                                context->current_time.second % 10);
}

char* days_of_week[]= {"NN on", "tu E ", "UU Ed", "th u ", "Fr i ", "SA t ", "Su n "};
void display_day_of_week(byte day) {//display
  vfdManager.update_char_array(days_of_week[day-1]);
}

void display_temperature() {//display and get temp
  float current_temp = context->temperatureManager.read_adc_to_celsius(current_millis);
  if (context->temperatureManager.temperature_unit == 'F') {
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
  vfdManager.displayed_characters[4] = context->temperatureManager.temperature_unit;
}*/

// This function runs when the board's sleep is interrupted by button 1 press
// Declare all variables modified by this as "volatile"
ISR(PCINT0_vect) {    // wake up
  // Flag to indicate first button press (waking up the board)
  // This button press should not be processed for normal state changes
  if (board_sleeping) {
    context->buttonManager.ignore_next_button_release = true;
    context->vfdManager.repower = true;
    board_sleeping = false;
    batteryReadingManager.battery_level = batteryReadingManager.BATTERY_READING;
    sleep_disable();
    ADCSRA = adcsra;
    control_state = DISPLAY_TIME;
    last_input_millis = current_millis;
    last_battery_read_millis = 0;
    TIMSK1 |= (1 << OCIE1A);
    
    digitalWrite(POWER_MEASURE_PIN, HIGH);
  }
}

void power_board_down(bool permit_wakeup) {//saving data, turning down GPIO pins, putting the board to sleep
  // Save current setting data
  switch (control_state) {
    case SETTING_HOUR:
      ds3231Manager.set_hour(setting_value, context->current_time);
      break;
    case SETTING_MINUTE:
      ds3231Manager.set_minute(setting_value, context->current_time);
      break;
    case SETTING_DAY_OF_WEEK:
      ds3231Manager.set_dayOfWeek(setting_value, context->current_time);
      break;
    case SETTING_DAY_OF_MONTH:
      ds3231Manager.set_dayOfMonth(setting_value, context->current_time);
      break;
    case SETTING_MONTH:
      ds3231Manager.set_month(setting_value, context->current_time);
      break;
    case SETTING_YEAR:
      ds3231Manager.set_year(setting_value, context->current_time);
      break;
    case SETTING_TEMPERATURE:
      //EEPROM.write(temperature_unit_eeprom_address, temperature_unit);
      context->temperatureManager.save_temp_unit();
      break;
  };

  leds.turn_off();
  context->vfdManager.turn_off();
  digitalWrite(POWER_MEASURE_PIN, LOW);
  board_sleeping = true;
  batteryReadingManager.battery_adc_sum = 0;
  batteryReadingManager.battery_adc_measurement_count = 0;
  // Set sleep wakeup interrupts
  if (permit_wakeup) {
    PCICR  |= 0b00000001;   // turn on port b for PCINTs
    PCMSK0 |= 0b00000100;   // turn on PCINT 2 mask
  } else {
    PCICR  = 0b00000000;    // turn off PCINT interrupt
    PCMSK0 = 0b00000000;
  }
  
  adcsra = ADCSRA;               //save the ADC Control and Status Register A
  ADCSRA = 0;                    //disable ADC
  mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE);  //turn off the brown-out detector
  mcucr2 = mcucr1 & ~_BV(BODSE);
  MCUCR = mcucr1;
  MCUCR = mcucr2;
  // Disable vfd heating timer interrupt
  TIMSK1 = 0;
  sleep_enable();
  sleep_cpu();                   //go to sleep
}

void flash_leds() {//output
  int led_millis = current_millis - wake_board_millis;
  if (led_millis < LOW_BATTERY_MESSAGE_DISPLAY_DURATION && led_millis % (2*LED_FLASH_INTERVAL) < LED_FLASH_INTERVAL) {
    leds.turn_on();
  } else if (led_millis < LOW_BATTERY_MESSAGE_DISPLAY_DURATION) {
    leds.turn_off();
  }
}

ISR(TIMER1_COMPA_vect){ //timer1 interrupt 50Hz toggles pin 5, 6
  context->vfdManager.heating();
}
