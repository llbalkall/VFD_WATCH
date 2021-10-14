#include "SimpleTime.h"

Time::Time()
{
  second = 0;
  minute = 0;
  hour = 0;
  dayOfWeek = 0;
  dayOfMonth = 0;
  month = 0;
  year = 0;
}

void Time::setTime(Time t)
{
  second = t.second;
  minute = t.minute;
  hour = t.hour;
  dayOfWeek = t.dayOfWeek;
  dayOfMonth = t.dayOfMonth;
  month = t.month;
  year = t.year;
}

void Time::setToZero()
{
  second = 0;
  minute = 0;
  hour = 0;
  dayOfWeek = 0;
  dayOfMonth = 0;
  month = 0;
  year = 0;
}

long Time::difference(Time t)
{
  long diff;
  diff += second - t.second 
        + (minute - t.minute) * 60 
        + (hour - t.hour) * 60 * 60 
        + (dayOfMonth - t.dayOfMonth) * 24 * 60 * 60; //when we are between two month at midnight this causes an error
  return diff;
}