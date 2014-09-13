#include "parser.h"

#include <sstream>

namespace SimpleStateMachine {

//---------------------------------------------------------------------------//

Line::Line(std::string _line)
	: line(_line) {
}

//---------------------------------------------------------------------------//

void Line::parseWhiteSpace() {
	while(line.size() > 0 && std::isspace(*line.begin())) {
		line.erase(line.begin());
	}
}

//---------------------------------------------------------------------------//

bool Line::parseWord(bool whitespaces) {
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

//---------------------------------------------------------------------------//

bool Line::parseParameterCall(bool whitespaces) {
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

//---------------------------------------------------------------------------//

bool Line::parseBool(bool whitespaces) {
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

//---------------------------------------------------------------------------//

bool Line::parseInteger(bool whitespaces) {
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

//---------------------------------------------------------------------------//

bool Line::parseFloat(bool whitespaces) {
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

//---------------------------------------------------------------------------//

bool Line::parseString(bool whitespaces) {
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

//---------------------------------------------------------------------------//

bool Line::parseFunction(bool whitespaces) {
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

//---------------------------------------------------------------------------//

char Line::parseAnySymbol(bool whitespaces) {
	if (line.size() == 0) return 0;
	char c = line.at(0);
	line.erase(line.begin());
	if (whitespaces) parseWhiteSpace();
	return c;
}

//---------------------------------------------------------------------------//

bool Line::parseSymbol(char c, bool whitespaces) {
	if (line.size() == 0) return false;
	if (line.at(0) != c) return false;
	parseAnySymbol(whitespaces);
	return true;
}

//---------------------------------------------------------------------------//

bool Line::parseSymbols(std::string symbols, bool whitespaces) {
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

//---------------------------------------------------------------------------//

bool Line::isEmpty() const {
	return (line.size() == 0);
}

//---------------------------------------------------------------------------//

std::string stripOffComment(std::string line) {
	auto pos = line.find('#');
	if (pos != std::string::npos) {
		line.erase(line.begin() + pos, line.end());
	}
	return line;
}

//---------------------------------------------------------------------------//

bool isEmptyLine(std::string const _line) {
	for (auto c : _line) {
		if (!std::isspace(c)) {
			return false;
		}
	}
	return true;
}

//---------------------------------------------------------------------------//

FunctionImage parseFunctionString(std::string _line, int callNr) {
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

//---------------------------------------------------------------------------//

std::tuple<bool, std::string> parseMachineString(std::string _line) {
	auto errorResult = std::make_tuple(false, "");
	Line line(_line);
	if(!line.parseWord(true)) return errorResult;
	if(!line.isEmpty()) return errorResult;

	return std::make_tuple(true, line.getResults().at(0));
}

//---------------------------------------------------------------------------//

std::tuple<bool, std::string, StateImage> parseStateString(std::string _line) {
	auto errorResult = std::make_tuple(false, "", StateImage());
	Line line(_line);
	if (!line.parseSymbol('\t')) return errorResult;

	std::string stateName;
	StateImage::ExecuteType executeType = StateImage::ExecuteType::Any;
	std::string actionName;
	FunctionImage functionImage;

	if (!line.parseWord(true)) return errorResult;
	stateName = line.getResults().at(0);

	if (!line.parseSymbol(':', true)) return errorResult;
	Line keywordLine(line.getRestLine());

	// This is actually not a while loop, just a if that I wanted to "break"
	while (keywordLine.parseWord(true)) {
		if (keywordLine.getResults().at(0) == "any") {
			executeType = StateImage::ExecuteType::Any;
		} else if (keywordLine.getResults().at(0) == "once") {
			executeType = StateImage::ExecuteType::Once;
		} else if (keywordLine.getResults().at(0) == "max_once") {
			executeType = StateImage::ExecuteType::MaxOnce;
		} else if (keywordLine.getResults().at(0) == "min_once") {
			executeType = StateImage::ExecuteType::MinOnce;
		} else break;
		line = Line(keywordLine.getRestLine());
		break;
	}
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

	StateImage stateImage {functionImage, executeType, {}, {}};
	return std::make_tuple(true, stateName, stateImage);
}

//---------------------------------------------------------------------------//

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

//---------------------------------------------------------------------------//

std::tuple<bool, std::string> parseSubMachineString(std::string _line) {
	auto errorResult = std::make_tuple(false, "");
	Line line(_line);
	if (!line.parseSymbols("\t\t*", true)) return errorResult;

	if (!line.parseWord(true)) return errorResult;
	if (line.getRestLine().size() > 0) return errorResult;

	return std::make_tuple(true, line.getResults().at(0));
}

//---------------------------------------------------------------------------//

}
