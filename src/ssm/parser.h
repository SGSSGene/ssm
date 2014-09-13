#ifndef SIMPLESTATEMACHINE_PARSER_H
#define SIMPLESTATEMACHINE_PARSER_H

#include <ssm/universeImage.h>

namespace SimpleStateMachine {

class Line {
private:
	std::string line;

	std::vector<std::string> results;
public:
	Line(std::string _line);

	std::vector<std::string> const& getResults()  const { return results; }
	std::string const&              getRestLine() const { return line; }

	void parseWhiteSpace   ();
	bool parseWord         (bool whitespaces=false);
	bool parseParameterCall(bool whitespaces=false);
	bool parseBool         (bool whitespaces=false);
	bool parseInteger      (bool whitespaces=false);
	bool parseFloat        (bool whitespaces=false);
	bool parseString       (bool whitespaces=false);
	bool parseFunction     (bool whitespaces=false);
	char parseAnySymbol    (bool whitespaces=false);
	bool parseSymbol       (char c, bool whitespaces=false);
	bool parseSymbols      (std::string symbols, bool whitespaces=false);
	bool isEmpty() const;
};

std::string                               stripOffComment      (std::string line);
bool                                      isEmptyLine          (std::string const _line);
FunctionImage                             parseFunctionString  (std::string _line, int callNr=0);
std::tuple<bool, std::string>             parseMachineString   (std::string _line);
std::tuple<bool, std::string, StateImage> parseStateString     (std::string _line);
std::tuple<bool, TransitionImage>         parseTransitionString(std::string _line);
std::tuple<bool, std::string>             parseSubMachineString(std::string _line);

}


#endif
