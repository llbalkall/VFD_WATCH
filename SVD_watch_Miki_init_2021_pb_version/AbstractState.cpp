#include "AbstractState.h"
#include "ConcreteStates.h"

AbstractState::~AbstractState(){

}

void AbstractState::set_context(Context *context) {
    this->context_ = context;
}

void AbstractState::both_held(){
   this->context_->TransitionTo(new StopWatch);
}

void AbstractState::first_held(){
    this->context_->TransitionTo(new StopWatch);
}

void AbstractState::second_held(){
    this->context_->TransitionTo(new EnterSettings);
}
