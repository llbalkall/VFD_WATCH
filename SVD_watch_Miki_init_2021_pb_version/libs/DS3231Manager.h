#ifndef DS3231MANAGER_H_INCLUDED
#define DS3231MANAGER_H_INCLUDED

#include <Wire.h>
#include <SimpleTime.h>
#define DS3231_I2C_ADDRESS 0x68

class DS3231Manager
{
public:
    void readDS3231time(unsigned char *second, unsigned char *minute, unsigned char *hour,
                        unsigned char *dayOfWeek, unsigned char *dayOfMonth, unsigned char *month, unsigned char *year);
    void setDS3231time(unsigned char second, unsigned char minute, unsigned char hour, unsigned char dayOfWeek,
                       unsigned char dayOfMonth, unsigned char month, unsigned char year);
    void set_minute(uint16_t value, Time time);
    void set_hour(uint16_t value, Time time);
    void set_dayOfWeek(uint16_t value, Time time);
    void set_dayOfMonth(uint16_t value, Time time);
    void set_month(uint16_t value, Time time);
    void set_year(uint16_t value, Time time);
};

int decToBcd(int val);
int bcdToDec(int val);

#endif
