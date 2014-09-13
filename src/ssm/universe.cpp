#include "universe.h"
#include "parser.h"

#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <tuple>

namespace SimpleStateMachine {

Universe::Universe() {
	conditionParaMap["true"] = [](ParameterImageList, TransitionImage const*)  { return []() { return true;  }; };
	conditionParaMap["else"] = [](ParameterImageList, TransitionImage const*)  { return []() { return true;  }; };
	conditionParaMap["false"] = [](ParameterImageList, TransitionImage const*) { return []() { return false; }; };
	ignoreConditions.insert("true");
	ignoreConditions.insert("else");
	ignoreConditions.insert("false");

	actionParaMap["__void__"] = [](ParameterImageList) { return []() {}; };
//!TODO __unknown__ should probably throw an error
	actionParaMap["__unknown__"] = [](ParameterImageList) { return []() {
		std::cout<<"unknown function is beeing called"<<std::endl;
	}; };
	ignoreActions.insert("");
	ignoreActions.insert("__void__");
	ignoreActions.insert("__unknown__");

}

Universe::Universe(std::istream& _istream, std::ostream& _ostream) {
	UniverseImage universeImage = parse(_istream);
	extractAllRequirements(universeImage);
	_ostream << "digraph {" << std::endl;
	// print all machines
	for (auto m : universeImage) {
		_ostream<<"\t";
		_ostream<<"subgraph cluster"<<m.first<<" {"<<std::endl;

		_ostream<<"\t\t";
		_ostream<<"label=\""<<m.first<<"\";"<<std::endl;

		_ostream<<"\t\tmachine_"<<m.first<<"_initialstate [label=\"\", shape=point, style=filled, fillcolor=\"#000000\"];"<<std::endl;
		_ostream<<"\t\tmachine_"<<m.first<<"_initialstate ->";
		_ostream<<"machine_"<<m.first<<"_state_"<<m.second.initialState<<";"<<std::endl;


		//print all states
		for (auto s : m.second.stateImageMap) {
			_ostream<<"\t\t";
			std::string executeType;
			if (s.second.executeType == StateImage::ExecuteType::Any) {
				executeType == "";
			} else if (s.second.executeType == StateImage::ExecuteType::Once) {
				executeType == "once";
			} else if (s.second.executeType == StateImage::ExecuteType::MaxOnce) {
				executeType == "max_once";
			} else if (s.second.executeType == StateImage::ExecuteType::MinOnce) {
				executeType == "min_once";
			}

			_ostream<<"machine_"<<m.first<<"_state_"<<s.first<<" [label=\""<<s.first<<executeType<<"\\n"<<s.second.function.decoratedFunction<<"\"];"<<std::endl;
		}
		_ostream<<std::endl;

		//print all condition->states
		for (auto s : m.second.stateImageMap) {
			for (auto t : s.second.transitionSet) {
				_ostream<<"\t\t";
				_ostream<<"machine_"<<m.first<<"_state_"<<s.first<<" -> ";
				_ostream<<"machine_"<<m.first<<"_state_"<<t.targetState;
				_ostream<<" [label=\""<<t.functionImage.decoratedFunction<<"\"];"<<std::endl;
			}
		}
		_ostream<<std::endl;

		//print sub machines
		for (auto s : m.second.stateImageMap) {
			for (auto subm : s.second.machineNameSet) {
				_ostream<<"\t\t";
				_ostream<<"machine_"<<m.first<<"cluster"<<subm;
				_ostream<<" [label=\""<<subm<<"\", shape=box];"<<std::endl;
			}
		}
		_ostream<<std::endl;

		//print connections to sub machines
		for (auto s : m.second.stateImageMap) {
			for (auto subm : s.second.machineNameSet) {
				_ostream<<"\t\t";
				_ostream<<"machine_"<<m.first<<"_state_"<<s.first<<" -> ";
				_ostream<<"machine_"<<m.first<<"cluster"<<subm<<" [style=dashed];"<<std::endl;
			}
		}
		_ostream<<std::endl;



		_ostream <<"\t}"<<std::endl;
	}
	_ostream << "}" << std::endl;
}
Universe::Universe(std::istream& _istream) {
	auto image = parse(_istream);
	extractAllRequirements(image);
}

void Universe::appendError(int _lineNbr, std::string _msg, std::string _line) {
	errorMessages << "Error in line "<<_lineNbr<<": "<<_msg<<" - " << _line<<std::endl;
}
Universe const& Universe::operator=(Universe const& _universe) {
	errorMessages.clear();
	errorMessages << _universe.errorMessages.str();

	requiredConditions       = _universe.requiredConditions;
	requiredActions          = _universe.requiredActions;

	ignoreActions            = _universe.ignoreActions;
	ignoreConditions         = _universe.ignoreConditions;

	actionParaMap            = _universe.actionParaMap;
	conditionParaMap         = _universe.conditionParaMap;

	return *this;
}
Universe const& Universe::operator=(Universe&& _universe) {
	errorMessages.clear();
	errorMessages << _universe.errorMessages.str();

	requiredActions          = std::move(_universe.requiredActions);
	requiredConditions       = std::move(_universe.requiredConditions);

	ignoreActions            = std::move(_universe.ignoreActions);
	ignoreConditions         = std::move(_universe.ignoreConditions);

	actionParaMap            = std::move(_universe.actionParaMap);
	conditionParaMap         = std::move(_universe.conditionParaMap);

	return *this;
}


UniverseImage Universe::parse(std::istream& _ifile) {
	UniverseImage universeImage;

	std::string line;
	std::getline(_ifile, line);

	enum state_t { Machine, State, Transition};

	state_t parseState (state_t::Machine);

	int lineNbr = 0;

	std::string currentMachineName = "";
	std::string currentStateName   = "";

	while(!_ifile.eof()) {
		lineNbr += 1;
		line = stripOffComment(line);

		if (!isEmptyLine(line)) {
			bool processed(false);
			if (parseState == state_t::Machine || parseState == state_t::State || parseState == state_t::Transition) {
				auto machine = parseMachineString(line);
				if (std::get<0>(machine)) {
					processed = true;
					currentMachineName = std::get<1>(machine);
					currentStateName   = "";
					universeImage[currentMachineName] = MachineImage();
					parseState = state_t::State;
				}
			}
			if (parseState == state_t::State || parseState == state_t::Transition) {
				auto state = parseStateString(line);
				if (std::get<0>(state)) {
					processed = true;
					currentStateName = std::get<1>(state);
					auto stateImage = std::get<2>(state);
					universeImage[currentMachineName].stateImageMap[currentStateName] = stateImage;
					if (universeImage[currentMachineName].initialState == "") {
						universeImage[currentMachineName].initialState = currentStateName;
					}
					parseState = state_t::Transition;
				}
			}
			if (parseState == state_t::Transition) {
				auto transition = parseTransitionString(line);
				if (std::get<0>(transition)) {
					processed = true;

					auto transitionImage = std::get<1>(transition);
					universeImage[currentMachineName].stateImageMap[currentStateName].transitionSet.push_back(transitionImage);
				}
				auto subMachine = parseSubMachineString(line);
				if (std::get<0>(subMachine)) {
					processed = true;

					auto subMachineName = std::get<1>(subMachine);
					universeImage[currentMachineName].stateImageMap[currentStateName].machineNameSet.push_back(subMachineName);
				}

			}
			if (!processed) {
				appendError(lineNbr, "Line couldn't be parsed", line);
			}
		}
		std::getline(_ifile, line);
	}
	return universeImage;
}
std::shared_ptr<Machine> Universe::bootstrap(std::string const& _ifile) {
	std::ifstream _istream(_ifile);
	return bootstrap(_istream);
}

std::shared_ptr<Machine> Universe::bootstrap(std::istream& _ifile) {
	auto image = parse(_ifile);
	extractAllRequirements(image);
	return bootstrap(image);
}
std::shared_ptr<Machine> Universe::bootstrap(UniverseImage const& _universeImage) {
	if (_universeImage.size() == 0) return std::shared_ptr<Machine>();
	auto a = actionParaMap;
	auto c = conditionParaMap;
	return bootstrap(_universeImage, a, c, "UniverseMachine");
}

std::shared_ptr<Machine> Universe::bootstrap(UniverseImage const& _universeImage, ActionParaMap _actionParaMap, ConditionParaMap _conditionParaMap, std::string const& _machineName) {
	if (_universeImage.find(_machineName) == _universeImage.end()) {
		appendError(0, "Machine name couldn't be found", _machineName);
		return std::shared_ptr<Machine>();
	}
	auto machineImage = _universeImage.at(_machineName);

	if (machineImage.stateImageMap.size() == 0) {
		appendError(0, "Machine without states not allowed", _machineName);
		return std::shared_ptr<Machine>();
	}
	// Create machine
	auto machine = new Machine();

	// create all States
	for (auto stateImage : machineImage.stateImageMap) {
		State* state = new State();
		auto actionName = stateImage.second.function.genericFunction;
		if (actionName == "") {
			actionName = "__void__";
		}
		if (_actionParaMap.find(actionName) == _actionParaMap.end()) {
			appendError(0, "Action couldn't be found", actionName);
			actionName = "__unknown__";
		}

		state->action = _actionParaMap.at(actionName)(stateImage.second.function.parameters);
		state->executeType = stateImage.second.executeType;
		machine->stateMap[stateImage.first] = std::shared_ptr<State>(state);
	}
	// set Initial State
	auto initialState = machine->stateMap.find(machineImage.initialState);
	if (initialState != machine->stateMap.end()) {
		machine->initialState = initialState->second;
	} else {
		appendError(0, "Couldn't find initial state", machineImage.initialState);
		machine->initialState = machine->stateMap.begin()->second;
	}

	// create all submachines
	auto oldConditionMap = _conditionParaMap;
	for (auto stateImage : machineImage.stateImageMap) {
		auto state = machine->stateMap.at(stateImage.first);
		for (auto machineName : stateImage.second.machineNameSet) {
			auto subMachine = bootstrap(_universeImage, _actionParaMap, oldConditionMap, machineName);
			if (!subMachine) {
				appendError(0, "Couldn't create sub machine", machineName);
				continue;
			}
			state->machineSet.push_back(subMachine);
			for (auto x : subMachine->stateMap) {
				std::string condition = machineName;
				condition.append(".").append(x.first);

				ignoreConditions.insert(condition);

				_conditionParaMap[condition] = [=](ParameterImageList, TransitionImage const*) {
					return [=]() {
						return subMachine->currentState.get() == x.second.get();
					};
				};
			}
		}
	}


	// create all Transitions
	for (auto stateImage : machineImage.stateImageMap) {
		auto state = machine->stateMap.at(stateImage.first);
		for (auto transitionImage : stateImage.second.transitionSet) {
			Transition* transition = new Transition();
			std::string conditionName = transitionImage.functionImage.genericFunction;
			auto parameters = transitionImage.functionImage.parameters;
			auto condition = _conditionParaMap.find(conditionName);
			if (condition != _conditionParaMap.end()) {
				transition->condition = condition->second(parameters, &transitionImage);
			} else {
				appendError(0, "Couldn't find condition symbol", conditionName);
				transition->condition = []() { return false; };
			}

			auto targetState = machine->stateMap.find(transitionImage.targetState);
			if (targetState != machine->stateMap.end()) {
				transition->targetState = targetState->second;
			} else {
				appendError(0, "Couldn't find target state", transitionImage.targetState);
				transition->condition = []() { return false; };
			}

			state->transitionSet.push_back(std::shared_ptr<Transition>(transition));
		}
	}
	machine->reset();
	return std::shared_ptr<Machine>(machine);
}

void Universe::extractAllRequirements(UniverseImage const& universeImage) {
	// Extract all conditions and actions
	for (auto m : universeImage) {
		for (auto s : m.second.stateImageMap) {
			if (s.second.function.decoratedFunction != "") {
				requiredActions[s.second.function.genericFunction] = s.second.function.signature;
			}
			for (auto t : s.second.transitionSet) {
				auto c = t.functionImage.genericFunction;

				//check if some keywords
				if (c == "true"
				    || c == "false"
					|| c == "else") {
					continue;
				}
				if (c.find('.') != std::string::npos) {
					std::string p1 = c;
					std::string p2 = c;
					p1.erase(p1.begin() + p1.find('.'), p1.end());
					p2.erase(p2.begin(), p2.begin() + p2.find('.')+1);
					auto _m = universeImage.find(p1);
					if (_m != universeImage.end()) {
						auto _s = _m->second.stateImageMap.find(p2);
						if (_s != _m->second.stateImageMap.end()) {
							continue;
						}
					}
				}
				requiredConditions[c] = t.functionImage.signature;
			}
		}
	}

}
}

