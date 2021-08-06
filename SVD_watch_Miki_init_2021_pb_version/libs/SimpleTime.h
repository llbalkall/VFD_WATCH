#ifndef SIMPLETIME_H_INCLUDED
#define SIMPLETIME_H_INCLUDED

class Time
{
public:
    // second, minute, hour, dayOfWeek, dayOfMonth, month, year
    char second;
    char minute;
    char hour;
    char dayOfWeek;
    char dayOfMonth;
    char month;
    char year;
    void setTime(Time t);
    Time();
    void setToZero();
};

#endif