#ifndef SIMPLETIME_H_INCLUDED
#define SIMPLETIME_H_INCLUDED

class Time{
    public:  
    // second, minute, hour, dayOfWeek, dayOfMonth, month, year
        unsigned char second;
        unsigned char minute;
        unsigned char hour;
        unsigned char dayOfWeek;
        unsigned char dayOfMonth;
        unsigned char month;
        unsigned char year;
        void setTime(Time t);
        Time();
};

#endif