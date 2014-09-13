#ifndef SIMPLESTATEMACHINE_STATE_H
#define SIMPLESTATEMACHINE_STATE_H

#include <ssm/action.h>
#include <ssm/universeImage.h>

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
	StateImage::ExecuteType executeType;
	bool fired;
	std::vector<std::shared_ptr<Transition>> transitionSet;
	std::vector<std::shared_ptr<Machine>>    machineSet;

	std::vector<Action const*> actionList; // Optimization hack
public:
	void reset();
	inline bool forceOneExecutionStep() const {
		return executeType == StateImage::ExecuteType::Once
		       || executeType == StateImage::ExecuteType::MinOnce;
	}
	std::shared_ptr<State> executeStep();
	inline std::vector<Action const*> const& getActionList() const {
		return actionList;
	}
	bool hasTransitions() const;
	friend class Universe;
};

}

#endif
