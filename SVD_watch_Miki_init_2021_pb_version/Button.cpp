#include "Button.h"
#include "Enums.h"
#include <Arduino.h>

Button::Button(int pin/*, ButtonEnum btnenumm*/)
{
    PIN = pin;
    hold_counts = 0;
    state = NOTHING_PRESSED;
    state_time = 0;
    released = false;
    //btnenum = btnenumm;
    pinMode(PIN, INPUT_PULLUP);
}

/**
 * Should be called frequently. Counts how many consecutive LOW was read on the PIN and
 * based on that decideds the state of the button. Could be either NOTHING_PRESSED, HELD, 
 * PRESSED_AND_RELEASED, HOLDING_STARTS. If the button is released until a treshold 
 * (BUTTON_HOLD_DURATION_MINIMUM) it's PRESSED_AND_RELEASED if it reaches the treshold it's 
 * HOLDING_STARTS. 
 * @return the state of the button, longpress, shortpress, or other not so important
 */
ButtonState Button::readState()
{
    if (state_time != millis()) // one per millisec is enough
    {
        released = false;
        if (digitalRead(PIN) == LOW)
        {
            hold_counts += 1;
            released = false;
        }
        else if (hold_counts > 0)
        {
            released = true;
            if (hold_counts < BUTTON_HOLD_DURATION_MINIMUM)
            {
                hold_counts = 0;
            }
        }
        if (hold_counts == BUTTON_HOLD_DURATION_MINIMUM)
        {
            state = HOLDING_STARTS;
            hold_counts++;
        }
        else if (released && hold_counts > BUTTON_HOLD_DURATION_MINIMUM)
        {
            state = NOTHING_PRESSED;
            hold_counts = 0;
        }
        else if (hold_counts > BUTTON_HOLD_DURATION_MINIMUM)
        {
            state = HELD;
        }
        else if (released)
        {
            state = PRESSED_AND_RELEASED;
        }
        else
        {
            state = NOTHING_PRESSED;
        }
    }
    state_time = millis();
    return state; // return natural stuff in case of not needing to compute stuff
}
