#include "AbstractState.h"
#include "ConcreteStates.h"

AbstractState::~AbstractState()
{
}

void AbstractState::set_context(Commander *commander)
{
    this->commander = commander;
}

void AbstractState::both_held()
{
  this->commander->setting_value = this->commander->party_mode_time_index;
  this->commander->TransitionTo(new SettingPartyModeName);
}

void AbstractState::top_held()
{
    this->commander->TransitionTo(new StopWatch);
}

void AbstractState::bottom_held()
{
    this->commander->TransitionTo(new EnterSettings);
}
