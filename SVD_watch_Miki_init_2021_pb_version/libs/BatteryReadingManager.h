#ifndef BATTERY_READING_H_INCLUDED
#define BATTERY_READING_H_INCLUDED

#define POWERSENSE_PIN A2

class BatteryReadingManager
{
private:
    const unsigned short BATTERY_READ_INTERVAL = 10;
    const unsigned short BATTERY_READ_MAX_COUNT = 15;

    const unsigned short HIGH_BATTERY_ADC_VALUE = 270;   //302;//3.9    //275 3.7;//285;//760;
    const unsigned short MEDIUM_BATTERY_ADC_VALUE = 260; //275;//3.7    //248 3.5;//269;
    const unsigned short LOW_BATTERY_ADC_VALUE = 240;    //251;//3.55   //221 3.3;//188;//254;

public:
    const int BATTERY_READING = 4;
    const int INSTANT_TURN_OFF = 3;
    const int TOO_LOW_FOR_DISPLAY = 2;
    const int GETTING_LOW = 1;
    const int DECENT = 0;

    unsigned long last_battery_read_millis;
    float battery_adc_measurement_count;
    int battery_level;
    float battery_adc_sum;

    BatteryReadingManager();
    void read_battery_level(unsigned long current_millis);
};

#endif