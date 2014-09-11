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

std::shared_ptr<State> State::executeStep(bool enter) {
	actionList.clear();

	for (auto const& machine : machineSet) {
		auto const& list = machine->_executeStep();
		if (list.size() > 1) {
			actionList.insert(actionList.end(), list.begin(), list.end());
		} else if (list.size() == 1) {
			actionList.push_back(list.at(0));
		}
	}

	if (!enter || !force) {
		for (auto const& transition : transitionSet) {
			if (transition->isEnabled()) {
				auto targetState(transition->getTargetState().lock());
				auto result(targetState->executeStep(true));
				actionList = targetState->getActionList();
				if (result) {
					return result;
				}
				return targetState;
			}
		}
	}
	if (!force || !fired) {
		actionList.insert(actionList.begin(), &action);
		fired = false;
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
