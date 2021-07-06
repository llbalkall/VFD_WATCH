#ifndef DS3231MANAGER_H_INCLUDED
#define DS3231MANAGER_H_INCLUDED


#include <Wire.h>
#define DS3231_I2C_ADDRESS 0x68

class DS3231Manager{
    public:
        void readDS3231time(unsigned char* second, unsigned char* minute, unsigned char* hour,
        unsigned char* dayOfWeek, unsigned char* dayOfMonth, unsigned char* month, unsigned char* year);
        void setDS3231time(unsigned char second, unsigned char minute, unsigned char hour, unsigned char dayOfWeek, 
        unsigned char dayOfMonth, unsigned char month, unsigned char year);
};


int decToBcd(int val);
int bcdToDec(int val);

#endif
