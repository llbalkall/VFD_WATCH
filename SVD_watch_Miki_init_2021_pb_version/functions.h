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
    
    int hold_counts_bottom;
    const int HOLD_DURATION_MINIMUM = 1000; // milliseconds
public:
    int hold_counts_top;
    unsigned short state;
    bool ignore_next_release;
    char ignore_next_releases;
    ButtonManager();
    void update_state();
    void wake_up_init();
    void first_wake_up_init();
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

#define BUZZER_PIN 3

class Buzzer
{
    private:
        bool is_on; 
    public: 
        Buzzer();
        void turn_on();
        void turn_off();
        void alarm(long millis);
};


#include <SimpleTime.h>
class Stopper
{
    public:
        Time start_time;
        int elapsed_sec;
        int state; //0:zeros, 1:running, 2:stopped
        Stopper();
        void set_elapsed_sec(int t);
        int get_elapsed_sec();
        void set_state(int state_);
        int get_state();
        void update_elapsed_sec(Time t);
};

#endif
