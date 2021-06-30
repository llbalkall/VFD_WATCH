#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

int add(int a, int b);  // Function prototype, its declaration

float celsius_to_fahrenheit(float celsius);

int decToBcd(int val);

int bcdToDec(int val);

#include <Wire.h>
#define DS3231_I2C_ADDRESS 0x68

class DS3231Manager{
    public:
        void readDS3231time(unsigned char* second, unsigned char* minute, unsigned char* hour,
        unsigned char* dayOfWeek, unsigned char* dayOfMonth, unsigned char* month, unsigned char* year);
        void setDS3231time(unsigned char second, unsigned char minute, unsigned char hour, unsigned char dayOfWeek, 
        unsigned char dayOfMonth, unsigned char month, unsigned char year);
};

#define LED_1_PIN A1
#define LED_2_PIN A0
#define LED_4_PIN 7
#define LED_3_PIN 8

class LEDs{
    public:
        LEDs();
        void turn_on();
        void turn_off();
};

#endif
