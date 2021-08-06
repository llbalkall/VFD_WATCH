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