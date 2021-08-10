#include "functions.h"
#include <Arduino.h>


// Function definition
int add(int a, int b)
{
  return a + b;
}

float celsius_to_fahrenheit(float celsius)
{
  float fh = (celsius * 9.0) / 5.0;
  fh += 32;
  return fh;
}

LEDs::LEDs()
{
  pinMode(LED_1_PIN, OUTPUT);
  pinMode(LED_2_PIN, OUTPUT);
  pinMode(LED_3_PIN, OUTPUT);
  pinMode(LED_4_PIN, OUTPUT);
  turn_off();
}

void LEDs::turn_on()
{
  digitalWrite(LED_1_PIN, HIGH);
  digitalWrite(LED_2_PIN, HIGH);
  digitalWrite(LED_3_PIN, HIGH);
  digitalWrite(LED_4_PIN, HIGH);
  is_on = true;
}

void LEDs::turn_off()
{
  digitalWrite(LED_1_PIN, LOW);
  digitalWrite(LED_2_PIN, LOW);
  digitalWrite(LED_3_PIN, LOW);
  digitalWrite(LED_4_PIN, LOW);
  is_on = false;
}

void LEDs::alarm(unsigned long millis){
  if (millis % 1000 % 125 < 62 && millis % 1000 < 500){
    if (!is_on) turn_on();
  } else {
    if (is_on) turn_off();
  }
}

ButtonManager::ButtonManager()
{
  pinMode(BUTTON_1_PIN, INPUT_PULLUP); // PCINT 1
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  hold_counts_top = 0;
  hold_counts_bottom = 0;
  state = 0;
  ignore_next_release = false;
  ignore_next_releases = 0;
}

void ButtonManager::first_wake_up_init(){
  /*if (hold_counts_top > 0 && hold_counts_bottom > 0){
  ignore_next_releases = 2;
  state = 0;
  } else if (hold_counts_top > 0) {
    ignore_next_releases = 1;
    state = 0;
  }  else if (hold_counts_top == 0 && hold_counts_bottom == 0){
    ignore_next_releases = 0;
    state = 0;
  }
  ignore_next_release = 0;
  state = 0;*/
  if (hold_counts_top > 0 && hold_counts_bottom > 0){
  ignore_next_releases = 2;
  state = 0;
  } else if (hold_counts_top > 0) {
    ignore_next_releases = 1;
    state = 0;
  }  else if (hold_counts_top == 0 && hold_counts_bottom == 0){
    ignore_next_releases = 0;
    state = 0;
  }
}

void ButtonManager::wake_up_init(){
  if (hold_counts_top > 0 && hold_counts_bottom > 0){
  ignore_next_releases = 2;
  state = 0;
  } else if (hold_counts_top > 0) {
    ignore_next_releases = 1;
    state = 0;
  }  else if (hold_counts_top == 0 && hold_counts_bottom == 0){
    ignore_next_releases = 0;
    state = 0;
  }
}

void ButtonManager::update_state()
{
  bool top_released = false;
  bool bottom_released = false;
  if (digitalRead(BUTTON_2_PIN) == LOW) hold_counts_top += 1;
  else if (hold_counts_top > 0) {
    top_released = true;
    hold_counts_top = 0;
  }
  if (digitalRead(BUTTON_1_PIN) == LOW) hold_counts_bottom += 1;
  else if (hold_counts_bottom > 0){
    bottom_released = true;
    hold_counts_bottom = 0;
  }
  // States: 0 - nothing pressed, 1 - #1 pressed and released, 2 - #2 pressed and released
  // 3 - #1 held, 4 - #2 held, 5 - both held
  if (hold_counts_top > HOLD_DURATION_MINIMUM &&
      hold_counts_bottom > HOLD_DURATION_MINIMUM) {
    state = 5;
    ignore_next_releases = 2;    
  } else if (hold_counts_bottom > HOLD_DURATION_MINIMUM &&
             hold_counts_top < 1) {
    if (ignore_next_releases == 2){
      state = 5;
    } else {
      state = 4;
      ignore_next_releases = 1;
    }
  } else if (hold_counts_top > HOLD_DURATION_MINIMUM &&
             hold_counts_bottom < 1) {
    if (ignore_next_releases == 2){
      state = 5;
    } else {
      state = 3;
      ignore_next_releases = 1;
    }
  } else if (bottom_released && ignore_next_releases == 0) {
    state = 2;
  } else if (bottom_released && ignore_next_releases != 0) {
    state = 0;
    ignore_next_releases = 0;
  } else if (top_released && ignore_next_releases == 0) {
    state = 1;
  } else if (top_released && ignore_next_releases != 0) {
    state = 0; 
    ignore_next_releases = 0;
  }
}

TemperatureManager::TemperatureManager()
{
  pinMode(TEMPERATURE_PIN, INPUT);
  temperature_unit = load_temp_unit();
  last_temp_read = 0;
  temperature_unit = 'C';
  temp = 0;
}

float TemperatureManager::read_adc_to_celsius(unsigned long current_millis)
{ //input
  if (current_millis - last_temp_read > TEMP_READ_INTERVAL || last_temp_read == 0)
  {
    last_temp_read = current_millis;
    temp = (analogRead(TEMPERATURE_PIN) * 3.3) / 1024.0;
    temp -= 0.5;        // Set to 0°C
    temp = temp / 0.01; // Scale 10 mv / °C
  }
  return temp;
}

void TemperatureManager::save_temp_unit()
{
  EEPROM.write(temperature_unit_eeprom_address, temperature_unit);
}

char TemperatureManager::load_temp_unit()
{
  return EEPROM.read(temperature_unit_eeprom_address);
}


Buzzer::Buzzer(){
  TCCR2A = _BV(COM2A0) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS22) | _BV(CS20);
  OCR2A = 18;
  OCR2B = 9;
  pinMode(BUZZER_PIN, INPUT);
  is_on = false;
}

void Buzzer::turn_on(){
  pinMode(BUZZER_PIN, OUTPUT);
  is_on = true;
}

void Buzzer::turn_off(){
  pinMode(BUZZER_PIN, INPUT);
  is_on = false;
}

void Buzzer::alarm(long millis){
  if (millis % 1000 % 125 < 62 && millis % 1000 < 500){
    if (!is_on) turn_on();
  } else {
    if (is_on) turn_off();
  }
}

Stopper::Stopper(){
  elapsed_sec = 0;
  state = 0;
}

void Stopper::set_elapsed_sec(int t){
  elapsed_sec = t;
}
int Stopper::get_elapsed_sec(){
  return elapsed_sec;
}
void Stopper::set_state(int state_){
  state = state_;
}
int Stopper::get_state(){
  return state;
}

void Stopper::update_elapsed_sec(Time t){
  elapsed_sec = t.difference(start_time);
}
