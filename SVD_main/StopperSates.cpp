#include "StopperStates.h"
#include "ConcreteStates.h"

void StopWatchRunning::update_display()
{
  this->commander->display_stopwatch();
  this->commander->stopper.update_elapsed_sec(this->commander->current_time);
}

void StopWatchRunning::top_pressed_and_released()
{
  this->commander->TransitionTo(new DisplayTime);
}

void StopWatchRunning::bottom_pressed_and_released()
{
    this->commander->stopper.set_state(2);
    this->commander->TransitionTo(new StopWatchStopped);
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
    this->commander->stopper.overflowed = false;
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




long BackToTheFutureAnimation::animation_millis(){
   return this->commander->current_millis - this->commander->bttf_animation_start_millis;
}

void BackToTheFutureAnimation::next_state(){
  this->commander->back_to_the_future_animation_state ++;
  reset_animation_time();
}

void BackToTheFutureAnimation::reset_animation_time(){
  this->commander->bttf_animation_start_millis = this->commander->current_millis;
}

int BackToTheFutureAnimation::single_line_animation(){
  int animation_state = animation_millis() / (single_animation_duration / 4)  % 4;
  switch (animation_state){
    case 0:
      this->commander->vfdManager.update_char_array("=    ");
      break;
    case 1:
      this->commander->vfdManager.update_char_array(" =   ");
      break;
    case 2:
      this->commander->vfdManager.update_char_array("   = ");
      break;
    case 3:
      this->commander->vfdManager.update_char_array("    =");
      break;
    default:
      break;
  }
  return animation_state;
} 

int BackToTheFutureAnimation::double_line_animation(){
  int animation_state = animation_millis()/(double_animation_duration / 4 )  % 4;
  switch (animation_state){
    case 0:
      this->commander->vfdManager.update_char_array("=   =");
      break;
    case 1:
      this->commander->vfdManager.update_char_array("==   ");
      break;
    case 2:
      this->commander->vfdManager.update_char_array(" = = ");
      break;
    case 3:
      this->commander->vfdManager.update_char_array("   ==");
      break;
    default:
      break;
  }
  return animation_state;
} 

int BackToTheFutureAnimation::full_line_animation(){
  int animation_state = animation_millis()/(full_animation_duration / 5)  % 5;
  switch (animation_state){
    case 0:
      this->commander->vfdManager.update_char_array("    ");
      break;
    case 1:
      this->commander->vfdManager.update_char_array("=    ");
      break;
    case 2:
      this->commander->vfdManager.update_char_array("==   ");
      break;
    case 3:
      this->commander->vfdManager.update_char_array("== = ");
      break;
    case 4:
      this->commander->vfdManager.update_char_array("== ==");
      break;
    default:
      break;
  }
  return animation_state;
}

void BackToTheFutureAnimation::update_display()
{
    switch (this->commander->back_to_the_future_animation_state)
    {
    case 0:  // show 1985
      this->commander->vfdManager.update_char_array("19 85");
      break;
    case 1:  //count up
      if (to_88 < 8800){
        to_88 = 8200 + animation_millis() / (10 / 3);
      } else {
        to_88 = 8800;
        next_state();
      }
      if (to_88>=4200){
        this->commander->vfdManager.disp_number(to_88);  
      } else{
        //fade 1985
        this->commander->vfdManager.update_char_array("     ");
      }
      break;
    case 2:  //show 88:00
      this->commander->vfdManager.disp_number(8800);
      if ((animation_millis() > 500 && animation_millis() < 700) ||
          (animation_millis() > 1000 && animation_millis() < 1200)
          ){
         this->commander->vfdManager.update_char_array("     ");
      } 
      if (animation_millis() > 2000){
        next_state();
        this->commander->back_to_the_future_animation_state = 5;
        this->commander->vfdManager.colon_steady = false;
      }
      break;
    case 3:  //
      single_line_animation();
      if (animation_millis() > single_animation_duration){
        if (single_animation_counter <= SINGLE_ANIMATION_NUMBER){
          single_animation_counter++;
          reset_animation_time();
        } else {
          next_state();
          single_animation_counter = 0;
        }
      }
      break;
    case 4:
      double_line_animation();
      if (animation_millis() > double_animation_duration){
        if (double_animation_counter <= DOUBLE_ANIMATION_NUMBER){
          double_animation_counter++;
          reset_animation_time();
        } else {
          next_state();
          double_animation_counter = 0;
        }
      }
      break;
    case 5:
      full_line_animation();

      if (animation_millis() > full_animation_duration){
        if (full_animation_counter <= FULL_ANIMATION_NUMBER){
          full_animation_counter++;
          reset_animation_time();
        } else {
          next_state();
          full_animation_counter = 0;
        }
      }
      break; 
    case 6:
    this->commander->vfdManager.update_char_array("== ==");
      this->commander->leds.animation_bttf(this->commander->current_millis);
      if (animation_millis() > 2000){
        next_state();
        this->commander->leds.turn_off();
      }
      break; 
    case 7:
      this->commander->vfdManager.update_char_array("19 55");
      if (animation_millis() > CHECKPOINT_3){
        next_state();
      }
      break;  
    default:
      break;
    }
}

void BackToTheFutureAnimation::top_pressed_and_released()
{
  if (this->commander->back_to_the_future_animation_state == 0){
    next_state();
    this->commander->bttf_animation_start_millis = this->commander->current_millis;
    this->commander->vfdManager.colon_steady = true;
  } else if (this->commander->back_to_the_future_animation_state > 6){
    this->commander->TransitionTo(new DisplayTime);
  }
}


void BackToTheFutureAnimation::bottom_pressed_and_released()
{
  top_pressed_and_released();
}
