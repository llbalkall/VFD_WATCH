#include "AbstractState.h"

AbstractState::~AbstractState(){

}

void AbstractState::set_context(Context *context) {
    this->context_ = context;
}
