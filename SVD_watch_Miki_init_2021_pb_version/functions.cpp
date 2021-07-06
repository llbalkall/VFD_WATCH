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