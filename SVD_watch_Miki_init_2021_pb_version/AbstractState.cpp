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
    this->commander->TransitionTo(new StopWatch);
}

void AbstractState::top_held()
{
    this->commander->TransitionTo(new StopWatch);
}

void AbstractState::bottom_held()
{
    this->commander->TransitionTo(new EnterSettings);
}
