#ifndef SIMPLESTATEMACHINE_MACHINE_H
#define SIMPLESTATEMACHINE_MACHINE_H

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <ssm/action.h>

namespace SimpleStateMachine {

class State;

class Machine {
private:
	std::map<std::string, std::shared_ptr<State>> stateMap;
	std::shared_ptr<State> currentState;
	std::shared_ptr<State> initialState;

	bool firstRun;

	std::vector<Action const*> const& _executeStep();
friend class State; // So it can call _executeStep()
public:
	void reset();
	void doTransition();
	bool executeStep();
	bool hasTransitions() const;

	friend class Universe;
};

}

#endif
