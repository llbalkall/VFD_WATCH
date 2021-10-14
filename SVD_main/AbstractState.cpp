#include "AbstractState.h"
#include "ConcreteStates.h"
#include "StopperStates.h"

AbstractState::~AbstractState()
{
}

void AbstractState::set_context(Commander *commander)
{
    this->commander = commander;
}

void AbstractState::both_held()
{
    this->commander->turn_alarm_off();
    this->commander->setting_value = this->commander->party_mode_time_index;
    this->commander->TransitionTo(new SettingPartyModeName);
}

void AbstractState::top_held()
{
    if (this->commander->current_time.year == 100){
        this->commander->TransitionTo(new BackToTheFutureAnimation);
        this->commander->back_to_the_future_animation_state = 0;
        this->commander->bttf_animation_start_millis = this->commander->current_millis;
    }   else {
        this->commander->turn_alarm_off();
        switch (this->commander->stopper.get_state())
        {
        case 0:
            this->commander->TransitionTo(new StopWatchNulled);
            break;
        case 1:
            this->commander->TransitionTo(new StopWatchRunning);
            break;
        case 2:
            this->commander->TransitionTo(new StopWatchStopped);
            break;
        default:
            this->commander->TransitionTo(new StopWatchNulled);
            break;
        }
    }
}
    

void AbstractState::bottom_held()
{
    this->commander->turn_alarm_off();
    this->commander->stopper.set_state(0);
    this->commander->TransitionTo(new EnterSettings);
}
