#include "universe.h"

#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <tuple>

namespace SimpleStateMachine {

std::string stripOffComment(std::string line) {
	auto pos = line.find('#');
	if (pos != std::string::npos) {
		line.erase(line.begin() + pos, line.end());
	}
	return line;
}

bool isEmptyLine(std::string const _line) {
	for (auto c : _line) {
		if (!std::isspace(c)) {
			return false;
		}
	}
	return true;
}
class Line {
private:
	std::string line;

	std::vector<std::string> results;
public:
	Line(std::string _line) : line(_line) {}

	std::vector<std::string> const& getResults() const { return results; }
	std::string const& getRestLine() const { return line; }

	void parseWhiteSpace() {
		while(line.size() > 0 && std::isspace(*line.begin())) {
			line.erase(line.begin());
		}
	}
	bool parseWord(bool whitespaces=false) {
		if (line.size() <= 0) return false;
		if (line.at(0) != '_' && !std::isalpha(line.at(0))) return false;
		std::string retStr = "";
		std::string procLine = line;

		for (auto c : line) {
			if (c != '_' && !std::isalnum(c) && c != '.') {
				break;
			}
			retStr += c;
			procLine.erase(procLine.begin());
		}

		line = procLine;
		results.clear();
		results.push_back(retStr);
		if (whitespaces) parseWhiteSpace();
		return true;
	}
	bool parseParameterCall(bool whitespaces=false) {
		Line _line(line);
		if (!_line.parseSymbol('(')) return false;

		int ct = 1;
		std::string retStr;
		while (ct > 0) {
			if(_line.isEmpty()) return false;
			if(_line.parseSymbol('(')) ++ct;
			else if (_line.parseSymbol(')')) --ct;
			else retStr += _line.parseAnySymbol();
		}

		line = _line.getRestLine();
		results.clear();
		results.push_back(retStr);
		if (whitespaces) parseWhiteSpace();
		return true;
	}
	bool parseBool(bool whitespaces=false) {
		Line parseBool(line);
		if (!parseBool.parseWord()) return false;
		if (parseBool.getResults().at(0) != "true" &&
		   parseBool.getResults().at(0) != "false") return false;

		results.clear();
		results.push_back(parseBool.getResults().at(0));
		line = parseBool.getRestLine();
		if (whitespaces) parseWhiteSpace();

		return true;
	}
	bool parseInteger(bool whitespaces=false) {
		std::string retStr = "";
		Line _line(line);
		std::string noSign = line;
		if (_line.parseSymbol('+', true))
			noSign = _line.getRestLine();
		else if (_line.parseSymbol('-', true)) {
			noSign = _line.getRestLine();
			retStr += '-';
		}

		if (noSign.size() <= 0) return false;
		if (!std::isdigit(noSign.at(0))) return false;

		for (auto c : noSign) {
//!TODO Hack to detect floats
			if (c == '.') return false;
			if (!std::isdigit(c)) break;
			retStr += _line.parseAnySymbol();
		}

		line = _line.getRestLine();
		results.clear();
		results.push_back(retStr);
		if (whitespaces) parseWhiteSpace();
		return true;

	}
	bool parseFloat(bool whitespaces=false) {
		std::string retStr = "";
		Line _line(line);
		std::string noSign = line;
		if (_line.parseSymbol('+', true))
			noSign = _line.getRestLine();
		else if (_line.parseSymbol('-', true)) {
			noSign = _line.getRestLine();
			retStr += '-';
		}

		if (noSign.size() <= 0) return false;
		if (!std::isdigit(noSign.at(0)) && (retStr == "-" || noSign.at(0) != '.') ) return false;

		std::string beforeStr, afterStr;
		bool before = true;
		for (auto c : noSign) {
			if (c == '.' && before) {
				before = false;
				_line.parseAnySymbol();
				continue;
			} else if (c == '.' && !before) {
				return false;
			}

			if (!std::isdigit(c)) break;
			if (before) beforeStr += _line.parseAnySymbol();
			else        afterStr  += _line.parseAnySymbol();
		}

		if (before) return false;
		if (beforeStr == "" && afterStr == "") return false;

		line = _line.getRestLine();
		results.clear();
		results.push_back(retStr + beforeStr + "." + afterStr);
		if (whitespaces) parseWhiteSpace();
		return true;

	}
	bool parseString(bool whitespaces=false) {
		Line _line(line);
		if (!_line.parseSymbol('"')) return false;
		std::string retStr = "";
		while (!_line.parseSymbol('"')) {
			if(_line.isEmpty()) return false;
			retStr += _line.parseAnySymbol();
		}

		line = _line.getRestLine();
		results.clear();
		results.push_back(retStr);
		if (whitespaces) parseWhiteSpace();
		return true;
	}
	bool parseFunction(bool whitespaces=false) {
		Line _line(line);
		if (!_line.parseWord(true)) return false;
		std::string retStr = _line.getResults().at(0);
		if (_line.parseParameterCall(true)) {
			std::string call = _line.getResults().at(0);
			Line callLine(call);
			if (!(callLine.isEmpty() || callLine.parseBool(true) || callLine.parseInteger(true) || callLine.parseFloat(true) || callLine.parseString(true))) return false;
			while (callLine.parseSymbol(',', true)) {
				if (!(callLine.parseBool(true) || callLine.parseInteger(true) || callLine.parseFloat(true) || callLine.parseString(true))) return false;
			}
			retStr += "(" +  call + ")";
		}
		if (_line.parseSymbol('.', true)) {
			retStr += std::string(".");
			if (!_line.parseFunction(true)) return false;
			retStr += _line.getResults().at(0);
		}

		line = _line.getRestLine();
		results.clear();
		results.push_back(retStr);
		if (whitespaces) parseWhiteSpace();
		return true;
	}
	char parseAnySymbol(bool whitespaces=false) {
		if (line.size() == 0) return 0;
		char c = line.at(0);
		line.erase(line.begin());
		if (whitespaces) parseWhiteSpace();
		return c;
	}
	bool parseSymbol(char c, bool whitespaces=false) {
		if (line.size() == 0) return false;
		if (line.at(0) != c) return false;
		parseAnySymbol(whitespaces);
		return true;
	}
	bool parseSymbols(std::string symbols, bool whitespaces=false) {
		Line _line(line);
		for (auto c : symbols) {
			if(!_line.parseSymbol(c)) return false;
		}
		line = _line.getRestLine();
		results.clear();
		results.push_back(symbols);
		if (whitespaces) parseWhiteSpace();
		return true;
	}
	bool isEmpty() const {
		return (line.size() == 0);
	}
};
FunctionImage parseFunctionString(std::string _line, int callNr=0) {
	FunctionImage f;
	Line line(_line);

	line.parseWord(true);
	f.decoratedFunction = line.getResults().at(0);
	f.genericFunction   = line.getResults().at(0);
	f.extendedFunction  = line.getResults().at(0);

	if (line.parseParameterCall(true)) {
		Line para(line.getResults().at(0));
		para.parseWhiteSpace();
		f.decoratedFunction += "(";
		f.extendedFunction  += "(";
		while (!para.isEmpty()) {
			ParameterImage parameter;

			if (para.parseBool(true)) {
				parameter.v_b = para.getResults().at(0) == "true";
				f.signature += "bool, ";

				f.decoratedFunction += para.getResults().at(0);
			} else if (para.parseInteger(true)) {
				std::istringstream ( para.getResults().at(0) ) >> parameter.v_i;
				f.signature += "int, ";

				f.decoratedFunction += para.getResults().at(0);
			} else if (para.parseFloat(true)) {
				std::istringstream ( para.getResults().at(0) ) >> parameter.v_f;
				f.signature += "double, ";

				f.decoratedFunction += para.getResults().at(0);
			} else if (para.parseString(true)) {
				parameter.v_s = para.getResults().at(0);
				f.signature += "std::string, ";

				f.decoratedFunction += "\\\""+para.getResults().at(0)+"\\\"";
			}
			f.parameters.pList.push_back(parameter);

			std::stringstream ss;
			ss<<"std::get<"<<callNr<<">(p)";
			f.extendedFunction  += ss.str();
			callNr += 1;

			if (!para.parseSymbol(',', true)) break;
			f.decoratedFunction += ", ";
			f.extendedFunction  += ", ";
		}
		f.decoratedFunction += ")";
		f.extendedFunction  += ")";
	}
	if (line.parseSymbol('.')) {
		FunctionImage subCall = parseFunctionString(line.getRestLine(), callNr);
		f.decoratedFunction += "." + subCall.decoratedFunction;
		f.extendedFunction  += "." + subCall.extendedFunction;
		f.genericFunction   += "_" + subCall.genericFunction;
		f.signature         += subCall.signature;
		f.broadSignature    = f.signature;
		for (auto p : subCall.parameters.pList) {
			f.parameters.pList.push_back(p);
		}
	}
	return f;
}
std::tuple<bool, std::string> parseMachineString(std::string _line) {
	auto errorResult = std::make_tuple(false, "");
	Line line(_line);
	if(!line.parseWord(true)) return errorResult;
	if(!line.isEmpty()) return errorResult;

	return std::make_tuple(true, line.getResults().at(0));
}
std::tuple<bool, std::string, StateImage> parseStateString(std::string _line) {
	auto errorResult = std::make_tuple(false, "", StateImage());
	Line line(_line);
	if (!line.parseSymbol('\t')) return errorResult;

	std::string stateName;
	bool force = false;
	std::string actionName;
	FunctionImage functionImage;

	if (!line.parseWord(true)) return errorResult;
	stateName = line.getResults().at(0);

	if (line.parseSymbol('!', true)) force = true;
	if (!line.parseSymbol(':', true)) return errorResult;
	if (line.parseFunction(true)) {
		actionName = line.getResults().at(0);
	}

	if (!line.isEmpty()) return errorResult;

	// Special keyword
	if (actionName == "_") {
		actionName = stateName;
		actionName.at(0) = std::tolower(actionName.at(0));
	}
	if (actionName != "") {
		functionImage = parseFunctionString(actionName);
		if (functionImage.signature.size() > 1) {
			functionImage.signature.erase(functionImage.signature.size()-1);
			functionImage.signature.erase(functionImage.signature.size()-1);
		}
	}
	functionImage.broadSignature = "void, "+functionImage.signature;
	functionImage.signature = "void(" + functionImage.signature + ")";

	StateImage stateImage {functionImage, force, {}, {}};
	return std::make_tuple(true, stateName, stateImage);
}

std::tuple<bool, TransitionImage> parseTransitionString(std::string _line) {
	auto errorResult = std::make_tuple(false, TransitionImage());
	Line line(_line);
	if (!line.parseSymbols("\t\t")) return errorResult;

	bool negate = false;

	std::string targetState;
	auto parameters = ParameterImageList();

	TransitionImage::DataType compareDataType = TransitionImage::DataType::Boolean;
	TransitionImage::Compare compareSymbol = TransitionImage::Compare::Equal;
	std::string compareValue  = "true";

	std::string returnType = "bool";

	if (line.parseSymbol('!', true)) {
		negate = true;
		compareValue = "false";
	}
	if (!line.parseFunction(true)) return errorResult;
	FunctionImage functionImage = parseFunctionString(line.getResults().at(0));

	if (negate) functionImage.decoratedFunction = "!" + functionImage.decoratedFunction;
	if (functionImage.signature.size() > 1) {
		functionImage.signature.erase(functionImage.signature.size()-1);
		functionImage.signature.erase(functionImage.signature.size()-1);
	}
	functionImage.broadSignature = functionImage.signature;
	functionImage.signature = "(" + functionImage.signature + ")";


	if (line.parseSymbols("==", true) && !negate) {
		compareSymbol = TransitionImage::Compare::Equal;
		if (line.parseString(true)) {
			compareValue = line.getResults().at(0);
			returnType = "std::string";
		} else if (line.parseFloat(true)) {
			compareValue = line.getResults().at(0);
			returnType = "double";
		} else if (line.parseInteger(true)) {
			compareValue = line.getResults().at(0);
			returnType = "int";
		} else if (line.parseBool(true)) {
			compareValue = line.getResults().at(0);
			returnType = "bool";
		} else return errorResult;
		functionImage.decoratedFunction += std::string(" == ") + "\\\"" + compareValue +"\\\"";
	} else if (line.parseSymbols("!=", true) && !negate) {
		compareSymbol = TransitionImage::Compare::Unequal;
		if (line.parseString(true)) {
			compareValue = line.getResults().at(0);
			returnType = "std::string";
		} else if (line.parseFloat(true)) {
			compareValue = line.getResults().at(0);
			returnType = "double";
		} else if (line.parseInteger(true)) {
			compareValue = line.getResults().at(0);
			returnType = "int";
		} else if (line.parseBool(true)) {
			compareValue = line.getResults().at(0);
			returnType = "bool";
		} else return errorResult;
		functionImage.decoratedFunction += std::string(" != ") + "\\\"" + compareValue + "\\\"";
	} else if (!negate && (line.parseSymbols(">=", true)
	                       || line.parseSymbols("<=", true)
	                       || line.parseSymbols(">", true)
	                       || line.parseSymbols("<", true))) {
		auto _compareSymbol = line.getResults().at(0);
		if (line.parseFloat(true)) {
			compareValue = line.getResults().at(0);
			functionImage.decoratedFunction += " " + _compareSymbol + " " +line.getResults().at(0);
			returnType = "double";
		} else if (line.parseInteger(true)) {
			compareValue = line.getResults().at(0);
			functionImage.decoratedFunction += " " + _compareSymbol + " " + line.getResults().at(0);
			returnType = "int";
		} else return errorResult;
		if (_compareSymbol == ">=") compareSymbol = TransitionImage::Compare::GreaterEqual;
		if (_compareSymbol == "<=") compareSymbol = TransitionImage::Compare::LessEqual;
		if (_compareSymbol == ">") compareSymbol = TransitionImage::Compare::Greater;
		if (_compareSymbol == "<") compareSymbol = TransitionImage::Compare::Less;


	} else {
		returnType = "bool";
	}

	if (!line.parseSymbols("->", true)) return errorResult;
	if (!line.parseWord(true))          return errorResult;
	if (line.getRestLine().size() > 0)  return errorResult;

	if (functionImage.broadSignature != "") {
		functionImage.broadSignature = returnType +", "+functionImage.broadSignature;
	} else {
		functionImage.broadSignature = returnType;
	}
	functionImage.signature = returnType += functionImage.signature;

	targetState = line.getResults().at(0);;
	TransitionImage transitionImage = {functionImage, targetState, compareDataType, compareSymbol, compareValue};
	return std::make_tuple(true, transitionImage);
}

std::tuple<bool, std::string> parseSubMachineString(std::string _line) {
	auto errorResult = std::make_tuple(false, "");
	Line line(_line);
	if (!line.parseSymbols("\t\t*", true)) return errorResult;

	if (!line.parseWord(true)) return errorResult;
	if (line.getRestLine().size() > 0) return errorResult;

	return std::make_tuple(true, line.getResults().at(0));
}
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
			_ostream<<"machine_"<<m.first<<"_state_"<<s.first<<" [label=\""<<s.first<<(s.second.force?"!":"")<<"\\n"<<s.second.function.decoratedFunction<<"\"];"<<std::endl;
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
		state->force  = stateImage.second.force;
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

