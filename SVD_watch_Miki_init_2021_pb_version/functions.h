#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED

int add(int a, int b);  // Function prototype, its declaration

float celsius_to_fahrenheit(float celsius);

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
