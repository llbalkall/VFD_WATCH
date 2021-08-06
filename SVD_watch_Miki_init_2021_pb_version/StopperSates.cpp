#include "StopperStates.h"
#include "ConcreteStates.h"

void StopWatchRunning::update_display()
{
  this->commander->display_stopwatch();
  //update elapsed seconds
  this->commander->stopper.update_elapsed_sec(this->commander->current_time);
}

void StopWatchRunning::top_pressed_and_released()
{
  this->commander->TransitionTo(new DisplayTime);
}

void StopWatchRunning::bottom_pressed_and_released()
{
    //stop, and save stuff
    this->commander->stopper.set_state(2);
    this->commander->TransitionTo(new StopWatchStopped);
    /*this->commander->stopwatch_running = false;
    this->commander->stop_watch_time.dayOfMonth = this->commander->current_time.dayOfMonth - this->commander->stop_watch_time.dayOfMonth;
    this->commander->stop_watch_time.hour = this->commander->current_time.hour - this->commander->stop_watch_time.hour;
    this->commander->stop_watch_time.minute = this->commander->current_time.minute - this->commander->stop_watch_time.minute;
    this->commander->stop_watch_time.second = this->commander->current_time.second - this->commander->stop_watch_time.second;
    */
}



void StopWatchStopped::update_display()
{
  this->commander->display_stopwatch();
  this->commander->vfdManager.colon_steady = true;
}

void StopWatchStopped::top_pressed_and_released()
{
  this->commander->TransitionTo(new DisplayTime);
}

void StopWatchStopped::bottom_pressed_and_released()
{
    //set stuff to zero
    //this->commander->stopwatch_running = false;
    //this->commander->stop_watch_time.setToZero();
    this->commander->stopper.set_state(0);
    this->commander->TransitionTo(new StopWatchNulled);
}


void StopWatchNulled::update_display()
{
    this->commander->display_stopwatch();
    this->commander->vfdManager.update_char_array("00100");
    this->commander->vfdManager.colon_steady = true;
}

void StopWatchNulled::top_pressed_and_released()
{
  this->commander->TransitionTo(new DisplayTime);
}


void StopWatchNulled::bottom_pressed_and_released()
{
    //start
    this->commander->stopper.start_time.setTime(this->commander->current_time);
    this->commander->stopper.set_state(1);
    this->commander->TransitionTo(new StopWatchRunning);
}
