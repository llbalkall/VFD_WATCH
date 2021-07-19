#include "BatteryReadingManager.h"
#include <Arduino.h>

BatteryReadingManager::BatteryReadingManager()
{
  pinMode(POWERSENSE_PIN, INPUT);
  last_battery_read_millis = 0;
  battery_adc_measurement_count = 0;
  battery_level = 0;
  battery_adc_sum = 0;
}

void BatteryReadingManager::read_battery_level(unsigned long current_millis)
{ //input
  // 5V = 1024, 0V = 0
  // Battery level: above 3.7V: 0, 3.7-3.5: 1, 3.5-3.3: 2, 3.3- 3
  if (current_millis - last_battery_read_millis > BATTERY_READ_INTERVAL || last_battery_read_millis == 0)
  {
    last_battery_read_millis = current_millis;
    unsigned int bat_level_adc = analogRead(POWERSENSE_PIN);
    if (battery_adc_measurement_count < BATTERY_READ_MAX_COUNT)
    {
      battery_level == BATTERY_READING;
      battery_adc_sum += bat_level_adc;
      battery_adc_measurement_count += 1;
      if (battery_adc_measurement_count == BATTERY_READ_MAX_COUNT)
        battery_adc_sum = battery_adc_sum / battery_adc_measurement_count;
    }
    else if (battery_adc_sum > HIGH_BATTERY_ADC_VALUE)
      battery_level = DECENT;
    else if (battery_adc_sum > MEDIUM_BATTERY_ADC_VALUE)
      battery_level = GETTING_LOW;
    else if (battery_adc_sum > LOW_BATTERY_ADC_VALUE)
      battery_level = TOO_LOW_FOR_DISPLAY;
    else
      battery_level = INSTANT_TURN_OFF;
  }
}