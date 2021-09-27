#include <Wire.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include "functions.h"
#include <SimpleTime.h>
#include <BatteryReadingManager.h>
#include "Commander.h"
#include "ConcreteStates.h"

#define POWERSENSE_PIN A2
#define BUZZER_PIN 3
#define POWER_MEASURE_PIN A7

const int SLEEP_TIMEOUT_INTERVAL = 12000;
const int SERIAL_NUMBER = 201;

void power_board_down(bool permit_wakeup);
//void flash_leds();

volatile bool board_sleeping = false;
bool stopwatch_running = false;
byte adcsra, mcucr1, mcucr2;
uint16_t setting_value = 0;

volatile unsigned long last_input_millis = 0;
unsigned long current_millis = 0;
volatile unsigned long last_battery_read_millis = 0;

BatteryReadingManager batteryManager;
Commander *commander = new Commander(new DisplayTime);
bool first_wake_up = true;

void setup()
{
  pinMode(POWER_MEASURE_PIN, OUTPUT);
  digitalWrite(POWER_MEASURE_PIN, LOW);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  power_board_down(true);
  commander->serial_number = SERIAL_NUMBER;
}

void loop()
{
  current_millis = millis();
  commander->current_millis = current_millis;
  if (batteryManager.level == batteryManager.BATTERY_READING)
  {
    batteryManager.read_level(current_millis);
  }
  else if (batteryManager.level == batteryManager.INSTANT_TURN_OFF)
  {
    power_board_down(false);
  }
  else
  {
    if (!board_sleeping && commander->vfdManager.repower)
    {
      commander->vfdManager.turn_on();
      if (batteryManager.level != batteryManager.TOO_LOW_FOR_DISPLAY)
      {
        digitalWrite(LOAD_PIN, HIGH); //TODO, what is this? VFD-s load pin
      }
      commander->vfdManager.repower = false;
      commander->wake_board_millis = current_millis;
      last_input_millis = current_millis;
    }
    if (batteryManager.level != batteryManager.TOO_LOW_FOR_DISPLAY)
    {
      commander->buttonManager.update_state();
      commander->read_current_time();
      commander->Update();
      if (batteryManager.level == batteryManager.GETTING_LOW && current_millis - commander->wake_board_millis < commander->LOW_BATTERY_MESSAGE_DISPLAY_DURATION)
      {
        commander->vfdManager.update_char_array("BA Lo");
      }
      //vfdManager.debug_4_digit(8888);
      //commander->vfdManager.update_char_aray("BB BB");
      
      commander->vfdManager.show_displayed_character_array(current_millis);
    }
    if (batteryManager.level == batteryManager.GETTING_LOW ||
        batteryManager.level == batteryManager.TOO_LOW_FOR_DISPLAY)
    {
      commander->flash_leds();
    }
    // End of interactive loop; all necessary input from button has been registered, reset it
    if (commander->buttonManager.state != 0)
      last_input_millis = current_millis;
    commander->buttonManager.state = 0;
    commander->vfdManager.colon_steady = false;
    if (commander->party_mode_is_on)
    {
      if (current_millis - last_input_millis > commander->PARTY_TIMES[commander->party_mode_time_index] * 1000 * 60 && !board_sleeping)
      {
        if (!commander->alarm_flag)
        {
          power_board_down(true);
        }
      }
    }
    else
    {
      if (commander->is_second_setting){
        if (current_millis - last_input_millis > 120000 && !board_sleeping){
          if (!commander->alarm_flag) power_board_down(true);
        }
      } else {
        if (current_millis - last_input_millis > SLEEP_TIMEOUT_INTERVAL && !board_sleeping){
          if (!commander->alarm_flag) power_board_down(true);
        }
      } 
    }
  }
}

// This function runs when the board's sleep is interrupted by button 1 press
ISR(PCINT0_vect)
{
}

void power_board_down(bool permit_wakeup)
{ // turning down GPIO pins, putting the board to sleep
  commander->leds.turn_off();
  commander->vfdManager.turn_off();
  commander->turn_alarm_off();
  commander->is_second_setting = false;
  digitalWrite(POWER_MEASURE_PIN, LOW);
  board_sleeping = true;
  batteryManager.adc_sum = 0;
  batteryManager.adc_measurement_count = 0;
  // Set sleep wakeup interrupts
  if (permit_wakeup)
  {
    PCICR |= 0b00000001;  // turn on port b for PCINTs
    PCMSK0 |= 0b00000110; // turn on PCINT 2 mask
  }
  else
  {
    PCICR = 0b00000000; // turn off PCINT interrupt
    PCMSK0 = 0b00000000;
  }
  //Set alarm interrupt
  PCICR |= (1 << PCIE1); // set PCIE2 to enable PCMSK2 scan
  PCMSK1 |= (1 << PCINT11);

  adcsra = ADCSRA;                         //save the ADC Control and Status Register A
  ADCSRA = 0;                              //disable ADC
  mcucr1 = MCUCR | _BV(BODS) | _BV(BODSE); //turn off the brown-out detector
  mcucr2 = mcucr1 & ~_BV(BODSE);
  MCUCR = mcucr1;
  MCUCR = mcucr2;
  // Disable vfd heating timer interrupt
  TIMSK1 = 0;
  sleep_enable();
  sleep_cpu(); //go to sleep

  if (board_sleeping)
  {
    commander->party_mode_is_on = false;
    commander->waking_up = true;
    commander->vfdManager.repower = true;
    
    board_sleeping = false;
    batteryManager.level = batteryManager.BATTERY_READING;
    sleep_disable();
    ADCSRA = adcsra;
    last_input_millis = current_millis;
    batteryManager.last_battery_read_millis = 0;
    TIMSK1 |= (1 << OCIE1A);
    digitalWrite(POWER_MEASURE_PIN, HIGH);
    commander->TransitionTo(new DisplayTime);
  }
}

ISR(TIMER1_COMPA_vect)
{ //timer1 interrupt 50Hz toggles pin 5, 6
  commander->vfdManager.heating();
}

ISR(PCINT1_vect)
{
  commander->trigger_alarm();
}
