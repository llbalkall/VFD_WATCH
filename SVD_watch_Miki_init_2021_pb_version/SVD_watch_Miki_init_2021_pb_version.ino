#include <Wire.h>
#include <avr/sleep.h>
#include <avr/interrupt.h> 
#include "functions.h"
#include <SimpleTime.h>
#include <BatteryReadingManager.h>
#include "Context.h"
#include "ConcreteStates.h"

#define POWERSENSE_PIN A2
#define BUZZER_PIN 3
#define POWER_MEASURE_PIN A7

const uint16_t SLEEP_TIMEOUT_INTERVAL = 25000;
const uint16_t LOW_BATTERY_MESSAGE_DISPLAY_DURATION = 2000;
const uint16_t LED_FLASH_INTERVAL = 150;

void read_current_time();
void power_board_down(bool permit_wakeup);
void display_stopwatch();
void flash_leds();

volatile bool board_sleeping = false;
bool stopwatch_running = false;
byte adcsra, mcucr1, mcucr2;
uint16_t setting_value = 0;

volatile unsigned long last_input_millis = 0;
volatile unsigned long wake_board_millis = 0;
unsigned long current_millis = 0;
volatile unsigned long last_battery_read_millis = 0;

BatteryReadingManager batteryReadingManager;
LEDs leds;
Context *context = new Context(new DisplayTime);

void setup() { //input, output init, setting up interrupts, timers
  pinMode(POWER_MEASURE_PIN, OUTPUT);
  digitalWrite(POWER_MEASURE_PIN, LOW);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  power_board_down(true);
}

void loop() {
  current_millis = millis();
  context->current_millis = current_millis;
  if (batteryReadingManager.battery_level == batteryReadingManager.BATTERY_READING){
     batteryReadingManager.read_battery_level(current_millis);
  } else if (batteryReadingManager.battery_level == batteryReadingManager.INSTANT_TURN_OFF) {
    power_board_down(false);
  } else {
    if (!board_sleeping && context->vfdManager.repower) {
      context->vfdManager.turn_on();
      if (batteryReadingManager.battery_level != batteryReadingManager.TOO_LOW_FOR_DISPLAY){
        digitalWrite(LOAD_PIN, HIGH);//TODO, what is this? VFD-s load pin
      } 
      context->vfdManager.repower = false;
      wake_board_millis = current_millis;
      last_input_millis = current_millis;
    }
    if (batteryReadingManager.battery_level != batteryReadingManager.TOO_LOW_FOR_DISPLAY) {
      context->buttonManager.update_button_state();
      read_current_time();
      context->Update();
      if (batteryReadingManager.battery_level == batteryReadingManager.GETTING_LOW && current_millis - wake_board_millis < LOW_BATTERY_MESSAGE_DISPLAY_DURATION) {
        context->vfdManager.update_char_array("BA Lo");
      }  
      //vfdManager.debug_4_digit(8888);
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
  context->ds3231Manager.readDS3231time(&context->current_time.second, &context->current_time.minute, &context->current_time.hour, &context->current_time.dayOfWeek,
  &context->current_time.dayOfMonth, &context->current_time.month, &context->current_time.year);
}

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
    context->TransitionTo(new DisplayTime);
    last_input_millis = current_millis;
    last_battery_read_millis = 0;
    TIMSK1 |= (1 << OCIE1A);
    
    digitalWrite(POWER_MEASURE_PIN, HIGH);
  }
}

void power_board_down(bool permit_wakeup) {// turning down GPIO pins, putting the board to sleep
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
