#ifndef BATTERY_READING_H_INCLUDED
#define BATTERY_READING_H_INCLUDED

#define POWERSENSE_PIN A2

class BatteryReadingManager
{
private:
    const unsigned short BATTERY_READ_INTERVAL = 10;
    const unsigned short BATTERY_READ_MAX_COUNT = 15;

    const unsigned short HIGH_BATTERY_ADC_VALUE = 266;   //3,7
    const unsigned short MEDIUM_BATTERY_ADC_VALUE = 260;  //3,6
    const unsigned short LOW_BATTERY_ADC_VALUE = 256;    //3,55


    /*
        3.44, 246
        3.55, 256
        3.58, 258
        3.66, 264
        3.75, 270
        3.85, 277
    */

   //302;//3.9    //275 3.7;//285;//760;
   //275;//3.7    //248 3.5;//269;
   //251;//3.55   //221 3.3;//188;//254;

public:
    const int BATTERY_READING = 4;
    const int INSTANT_TURN_OFF = 3;
    const int TOO_LOW_FOR_DISPLAY = 2;
    const int GETTING_LOW = 1;
    const int DECENT = 0;

    unsigned long last_battery_read_millis;
    float adc_measurement_count;
    int level;
    float adc_sum;

    BatteryReadingManager();
    void read_level(unsigned long current_millis);
};

#endif