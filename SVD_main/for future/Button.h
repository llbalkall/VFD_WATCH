#ifndef BUTTON_H_INCLUDED
#define BUTTON_H_INCLUDED

enum ButtonState
{
    NOTHING_PRESSED,
    PRESSED_AND_RELEASED,
    HOLDING_STARTS,
    HELD
};

const int BUTTON_HOLD_DURATION_MINIMUM = 500;

class Button
{
public:
    int PIN;
    int hold_counts;
    ButtonState state;
    long state_time;
    bool released;
    //ButtonEnum btnenum;

public:
    Button(int pin/*, ButtonEnum btnenumm*/);
    ButtonState readState();
};

#endif
