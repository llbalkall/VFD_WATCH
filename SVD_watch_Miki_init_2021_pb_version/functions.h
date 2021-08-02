#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

int add(int a, int b); // Function prototype, its declaration

float celsius_to_fahrenheit(float celsius);

#define LED_1_PIN A1
#define LED_2_PIN A0
#define LED_4_PIN 7
#define LED_3_PIN 8

class LEDs
{
public:
    LEDs();
    void turn_on();
    void turn_off();
};

#include <Arduino.h>
#define BUTTON_1_PIN 9
#define BUTTON_2_PIN 10

class ButtonManager
{
private:
    int button_hold_counts_1;
    int button_hold_counts_2;
    const int BUTTON_HOLD_DURATION_MINIMUM = 1000; // milliseconds
public:
    unsigned short button_state;
    bool ignore_next_button_release;
    char ignore_next_button_releases;
    ButtonManager();
    void update_button_state();
};

#define TEMPERATURE_PIN A6
#include <EEPROMex.h>
#include <EEPROMVar.h>

class TemperatureManager
{
private:
    const uint16_t TEMP_READ_INTERVAL = 5000;
    const int temperature_unit_eeprom_address = 16;

public:
    char temperature_unit;
    unsigned long last_temp_read;
    short counter;
    float temp;
    TemperatureManager();
    float read_adc_to_celsius(unsigned long current_millis);
    void save_temp_unit();
    char load_temp_unit();
};

#endif
