#include "functions.h"
#include <Arduino.h>

void debug_4_digit(float n, char* vfd_displayed_characters){
  /*int tens = 0;
  while (n>=10){
    n = n/10.0;
    tens++;
  }
  vfd_displayed_characters[0] = int(n) % 10;
  vfd_displayed_characters[1] = int(n*10) % 10;
  vfd_displayed_characters[2] = ' ';
  vfd_displayed_characters[3] = int (tens/10) % 10; 
  vfd_displayed_characters[4] = int(n) % 10;*/
  if (n>9999.9999){
    uint8_t n_tenthousands = int(n / 10000) % 10;
    uint8_t n_thousands = int(n / 1000) % 10;
    uint8_t n_hundreds = int(n / 100) % 10;
    uint8_t n_tens     = int(n / 10) % 10;
    uint8_t n_ones     = int(n)  % 10;
    vfd_displayed_characters[0] = n_tenthousands;
    vfd_displayed_characters[1] = n_thousands;
    vfd_displayed_characters[2] = 1;
    vfd_displayed_characters[3] = n_hundreds; 
    vfd_displayed_characters[4] = n_tens;
  } else {
    uint8_t n_thousands = int(n / 1000) % 10;
    uint8_t n_hundreds = int(n / 100) % 10;
    uint8_t n_tens     = int(n / 10) % 10;
    uint8_t n_ones     = int(n)  % 10;
    vfd_displayed_characters[0] = n_thousands;
    vfd_displayed_characters[1] = n_hundreds;
    vfd_displayed_characters[2] = ' ';
    vfd_displayed_characters[3] = n_tens; 
    vfd_displayed_characters[4] = n_ones;  
  }
}
/*
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

int decToBcd(int val) {
  return( (val/10*16) + (val%10) );
}

int bcdToDec(int val) {
  return( (val/16*10) + (val%16) );
}

void DS3231Manager::readDS3231time(unsigned char* second, unsigned char* minute, unsigned char* hour,
        unsigned char* dayOfWeek, unsigned char* dayOfMonth, unsigned char* month, unsigned char* year) {//DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set DS3231 register pointer to 00h
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  // request seven bytes of data from DS3231 starting from register 00h
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

void DS3231Manager::setDS3231time(unsigned char second, unsigned char minute, unsigned char hour, unsigned char dayOfWeek, 
        unsigned char dayOfMonth, unsigned char month, unsigned char year) {//DS3231

  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0); // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour)); // set hours
  Wire.write(decToBcd(dayOfWeek));
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month)); // set month
  Wire.write(decToBcd(year)); // set year (0 to 99)
  Wire.endTransmission();
}
/*
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
}*/
