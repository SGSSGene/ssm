#include "state.h"

#include "transition.h"
#include "machine.h"

namespace SimpleStateMachine {
void State::reset() {
	for (auto machine : machineSet) {
		machine->reset();
	}
	fired = false;
}

std::shared_ptr<State> State::executeStep() {
	actionList.clear();

	for (auto const& machine : machineSet) {
		auto const& list = machine->_executeStep();
		if (list.size() > 1) {
			actionList.insert(actionList.end(), list.begin(), list.end());
		} else if (list.size() == 1) {
			actionList.push_back(list.at(0));
		}
	}

	if (fired || !forceOneExecutionStep()) {
		for (auto const& transition : transitionSet) {
			if (transition->isEnabled()) {
				auto targetState(transition->getTargetState().lock());
				targetState->reset();
				auto result(targetState->executeStep());
				actionList = targetState->getActionList();
				if (result) {
					return result;
				}
				return targetState;
			}
		}
	}
	if (!fired
	    || executeType == StateImage::ExecuteType::Any
	    || executeType == StateImage::ExecuteType::MinOnce) {
		actionList.insert(actionList.begin(), &action);
		fired = true;
	}
	return std::shared_ptr<State>();
}
bool State::hasTransitions() const {
	if (transitionSet.size() > 0) return true;
	for (auto const& s : machineSet) {
		if (s->hasTransitions()) return true;
	}
	return false;
}


}
