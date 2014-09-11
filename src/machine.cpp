#include "machine.h"

#include "state.h"

#include "transition.h"

namespace SimpleStateMachine {

void Machine::reset() {
	currentState = initialState;
	currentState->reset();
	firstRun = true;
}
std::vector<Action const*> const& Machine::_executeStep() {
	auto oldState = currentState;
	auto result = currentState->executeStep(firstRun);
	firstRun = false;
	if (result) {
		currentState = result;
	}

	return oldState->getActionList();
}
bool Machine::executeStep() {
	auto const& actionList = _executeStep();
	for (auto const& a : actionList) {
		(*a)();
	}
	return hasTransitions();
}
bool Machine::hasTransitions() const {
	return currentState->hasTransitions();
}

}
