#include "functions.h"
#include <Arduino.h>

// Function definition
int add(int a, int b)
{
    return a + b;
}

float celsius_to_fahrenheit(float celsius) {
  float fh = (celsius * 9.0) / 5.0;
  fh += 32;
  return fh;
}

LEDs::LEDs(){
    pinMode(LED_1_PIN, OUTPUT);
    pinMode(LED_2_PIN, OUTPUT);
    pinMode(LED_3_PIN, OUTPUT);
    pinMode(LED_4_PIN, OUTPUT);
}

void LEDs::turn_on(){
  digitalWrite(LED_1_PIN, HIGH);
  digitalWrite(LED_2_PIN, HIGH);
  digitalWrite(LED_3_PIN, HIGH);
  digitalWrite(LED_4_PIN, HIGH);
}

void LEDs::turn_off(){
  digitalWrite(LED_1_PIN, LOW);
  digitalWrite(LED_2_PIN, LOW);
  digitalWrite(LED_3_PIN, LOW);
  digitalWrite(LED_4_PIN, LOW);
}

ButtonManager::ButtonManager(){
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);   // PCINT 1
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);
  button_hold_counts_1 = 0;
  button_hold_counts_2 = 0;
  button_state = 0;
  ignore_next_button_release = false;
}

void ButtonManager::update_button_state(){
  bool button_1_released = false;
  bool button_2_released = false;
  if (digitalRead(BUTTON_2_PIN) == LOW) button_hold_counts_1 += 1;
  else if (button_hold_counts_1 > 0) {
    button_1_released = true;
    button_hold_counts_1 = 0;
  }
  if (digitalRead(BUTTON_1_PIN) == LOW) button_hold_counts_2 += 1;
  else if (button_hold_counts_2 > 0){
    button_2_released = true;
    button_hold_counts_2 = 0;
  }
  // States: 0 - nothing pressed, 1 - #1 pressed and released, 2 - #2 pressed and released
  // 3 - #1 held, 4 - #2 held, 5 - both held
  if (button_hold_counts_1 > BUTTON_HOLD_DURATION_MINIMUM &&
      button_hold_counts_2 > BUTTON_HOLD_DURATION_MINIMUM) {
    button_state = 5;
    ignore_next_button_release = true;    
  } else if (button_hold_counts_2 > BUTTON_HOLD_DURATION_MINIMUM) {
    button_state = 4;
    ignore_next_button_release = true;
  } else if (button_hold_counts_1 > BUTTON_HOLD_DURATION_MINIMUM) {
    button_state = 3;
    ignore_next_button_release = true;
  } else if (button_2_released && !ignore_next_button_release) {
    button_state = 2;
  } else if (button_2_released && ignore_next_button_release) {
    button_state = 0;
    ignore_next_button_release = false;
  } else if (button_1_released && !ignore_next_button_release) {
    button_state = 1;
  } else if (button_1_released && ignore_next_button_release) {
    button_state = 0;
    ignore_next_button_release = false;
  }
}

TemperatureManager::TemperatureManager(){
  pinMode(TEMPERATURE_PIN, INPUT);
  temperature_unit = load_temp_unit();
  last_temp_read = 0;
  temperature_unit = 'C';
  temp = 0;
}

float TemperatureManager::read_adc_to_celsius(unsigned long current_millis) {//input
  if (current_millis - last_temp_read > TEMP_READ_INTERVAL || last_temp_read == 0) {
    last_temp_read = current_millis;
    temp = (analogRead(TEMPERATURE_PIN) * 3.3) / 1024.0;
    temp -= 0.5;         // Set to 0°C
    temp = temp / 0.01;  // Scale 10 mv / °C  
  }
  return temp;
}

void TemperatureManager::save_temp_unit(){
  EEPROM.write(temperature_unit_eeprom_address, temperature_unit);
}

char TemperatureManager::load_temp_unit(){
  return EEPROM.read(temperature_unit_eeprom_address);
}
