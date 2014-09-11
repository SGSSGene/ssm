#ifndef SIMPLESTATEMACHINE_STATE_H
#define SIMPLESTATEMACHINE_STATE_H

#include "action.h"

#include "action.h"
#include "time.h"

#include <list>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace SimpleStateMachine {

class Transition;
class Machine;

class State {
private:
	Action action;
	bool force;
	bool fired;
	std::vector<std::shared_ptr<Transition>> transitionSet;
	std::vector<std::shared_ptr<Machine>>    machineSet;

	std::vector<Action const*> actionList; // Optimization hack
public:
	void reset();
	inline bool forceOneExecutionStep() const {
		return force;
	}
	std::shared_ptr<State> executeStep(bool enter=false);
	inline std::vector<Action const*> const& getActionList() const {
		return actionList;
	}
	bool hasTransitions() const;
	friend class Universe;
};

}

#endif
