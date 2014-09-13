#ifndef SIMPLESTATEMACHINE_TRANSITION_H
#define SIMPLESTATEMACHINE_TRANSITION_H

#include "condition.h"

#include <memory>

namespace SimpleStateMachine {

class State;

class Transition {
private:
	Condition            condition;
	std::weak_ptr<State> targetState;

public:
	inline bool isEnabled() const { return condition(); }
	inline std::weak_ptr<State> const& getTargetState() const { return targetState; }

	friend class Universe;
};

}

#endif
