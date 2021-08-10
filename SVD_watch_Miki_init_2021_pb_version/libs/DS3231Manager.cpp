#include "DS3231Manager.h"

int decToBcd(int val)
{
  return ((val / 10 * 16) + (val % 10));
}

int bcdToDec(int val)
{
  return ((val / 16 * 10) + (val % 16));
}

void DS3231Manager::readDS3231time(unsigned char *second, unsigned char *minute, unsigned char *hour,
                                   unsigned char *dayOfWeek, unsigned char *dayOfMonth, unsigned char *month, unsigned char *year)
{ //DS3231
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
                                    unsigned char dayOfMonth, unsigned char month, unsigned char year)
{ //DS3231

  // sets time and date data to DS3231
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0);                // set next input to start at the seconds register
  Wire.write(decToBcd(second)); // set seconds
  Wire.write(decToBcd(minute)); // set minutes
  Wire.write(decToBcd(hour));   // set hours
  Wire.write(decToBcd(dayOfWeek));
  Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
  Wire.write(decToBcd(month));      // set month
  Wire.write(decToBcd(year));       // set year (0 to 99)
  Wire.endTransmission();
}

void DS3231Manager::clearAlarmStatusBits(){
  int buffer = readRTCRegister(0x0f);
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0x0f);
  Wire.write(0b11111100 & buffer); 
  Wire.endTransmission();
}

int DS3231Manager::readRTCRegister(int address){
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 1);
  return Wire.read();
}

//TODO this isn't the place for this
void DS3231Manager::writeRTCRegister(int address, int value){
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(address);
  Wire.write(value); 
  Wire.endTransmission();
}

void DS3231Manager::set_minute(uint16_t value, Time time)
{
  setDS3231time(0, value, time.hour, time.dayOfWeek, time.dayOfMonth, time.month, time.year);
}

void DS3231Manager::set_hour(uint16_t value, Time time)
{
  setDS3231time(0, time.minute, value, time.dayOfWeek, time.dayOfMonth, time.month, time.year);
}

void DS3231Manager::set_dayOfWeek(uint16_t value, Time time)
{
  setDS3231time(time.second, time.minute, time.hour, value, time.dayOfMonth, time.month, time.year);
}

void DS3231Manager::set_dayOfMonth(uint16_t value, Time time)
{
  setDS3231time(time.second, time.minute, time.hour, time.dayOfWeek, value, time.month, time.year);
}

void DS3231Manager::set_month(uint16_t value, Time time)
{
  setDS3231time(time.second, time.minute, time.hour, time.dayOfWeek, time.dayOfMonth, value, time.year);
}

void DS3231Manager::set_year(uint16_t value, Time time)
{
  setDS3231time(time.second, time.minute, time.hour, time.dayOfWeek, time.dayOfMonth, time.month, value);
}