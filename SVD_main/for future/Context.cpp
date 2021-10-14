#include "Context.h"

Context::Context(AbstractState *state) : state_(nullptr) {
    this->TransitionTo(state);
}

Context::~Context() {
    delete state_;
}

void Context::TransitionTo(AbstractState *state) {
    if (this->state_ != nullptr)
      delete this->state_;
    this->state_ = state;
    this->state_->set_context(this);
}

void Context::Request1() {
    this->state_->Handle1();
}

void Context::Request2() {
    this->state_->Handle2();
}
