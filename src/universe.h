#ifndef SIMPLESTATEMACHINE_UNIVERSE_H
#define SIMPLESTATEMACHINE_UNIVERSE_H


#include "state.h"
#include "transition.h"
#include "machine.h"
#include "universeImage.h"

#include <sstream>
#include <iostream>

namespace SimpleStateMachine {

class Universe {
private:
	std::stringstream errorMessages;

	std::map<std::string, std::string> requiredActions;
	std::map<std::string, std::string> requiredConditions;

	std::set<std::string> ignoreActions;
	std::set<std::string> ignoreConditions;

	ActionParaMap    actionParaMap;
	ConditionParaMap conditionParaMap;
public:
	Universe();
	Universe(std::istream& _istream, std::ostream& _ostream);
	Universe(std::istream& _istream);
	template<typename T>
	void autoRegister(T* t) {
		_autoRegister(this, t);
	}

	template <typename... Parameters>
	inline bool reg_action(std::string _name, delegate<void(std::tuple<Parameters...>)> _function);

	template <typename R, typename... Parameters>
	inline bool reg_condition(std::string _name, delegate<R(std::tuple<Parameters...>)> _function);

	std::string getErrorMessages() const { return errorMessages.str(); }

	void appendError(int _lineNbr, std::string _msg, std::string _line);

	Universe const& operator=(Universe const& _universe);
	Universe const& operator=(Universe&& _universe);

	std::map<std::string, std::string> const& getRequiredActions()    const { return requiredActions; }
	std::map<std::string, std::string> const& getRequiredConditions() const { return requiredConditions; }
	std::set<std::string> const& getIgnoreActions()    const { return ignoreActions; }
	std::set<std::string> const& getIgnoreConditions() const { return ignoreConditions; }


	UniverseImage parse(std::istream& _ifile);
	std::shared_ptr<Machine> bootstrap(std::string const& _ifile);
	std::shared_ptr<Machine> bootstrap(std::istream& _ifile);
	std::shared_ptr<Machine> bootstrap(UniverseImage const& _universeImage);
private:
	std::shared_ptr<Machine> bootstrap(UniverseImage const& _universeImage, ActionParaMap _actionParaMap, ConditionParaMap _conditionParaMap, std::string const& _machineName);

	void extractAllRequirements(UniverseImage const& universeImage);
};

template <typename... Args>
struct PartialBind_Impl;

template <typename R, typename First, typename ...Args>
struct PartialBind_Impl<R, First, Args...> {
	static std::tuple<First, Args...> partialBind_impl(delegate<R(std::tuple<First, Args...>)> _f, ParameterImageList* parameters) {

		auto value = parameters->pList.at(0).getValue<First>();
		parameters->pList.erase(parameters->pList.begin());

		auto f = delegate<R(std::tuple<Args...>)>();
		auto tuple_parameters = PartialBind_Impl<R, Args...>::partialBind_impl(f, parameters);
		auto tuple_parameters_full = std::tuple_cat(std::make_tuple(value), tuple_parameters);
		return tuple_parameters_full;
	}
};
template <typename R, typename First>
struct PartialBind_Impl<R, First> {
	static std::tuple<First> partialBind_impl(delegate<R(std::tuple<First>)> _f, ParameterImageList* parameters) {
		auto value = parameters->pList.at(0).getValue<First>();
		std::tuple<First> tuple_parameters = std::make_tuple(value);
		parameters->pList.erase(parameters->pList.begin());

		return tuple_parameters;
	}
};


template<typename R>
struct PartialBind_Impl<R> {
	static std::tuple<> partialBind_impl(delegate<R(std::tuple<>)> _f, ParameterImageList* parameters) {
		std::tuple<> tuple_parameters;
		return tuple_parameters;
	}
};
template <typename R, typename ...Args>
delegate<R()> partialBind(delegate<R(std::tuple<Args...>)> _f, ParameterImageList parameters) {
	auto para_list = PartialBind_Impl<R, Args...>::partialBind_impl(_f, &parameters);
	return std::bind(_f, para_list);
}

template <typename... Parameters>
inline bool Universe::reg_action(std::string _name, delegate<void(std::tuple<Parameters...>)> _function) {
	if (!_function) return false;
	actionParaMap[_name] = [_function](ParameterImageList _list) {
//!TODO ASSERT needed if wrong number of parameters is given
		return delegate<void()>(partialBind(_function, _list));
	};
	return true;
}

template<typename R>
struct Check {
static delegate<bool(R)> getCheck(TransitionImage const& t);
};

template<>
struct Check<bool> {
static delegate<bool(bool)> getCheck(TransitionImage const& t) {
	// ASSERT if wrong type

	bool value = (t.compareValue=="true");
	if (t.compareSymbol == TransitionImage::Compare::Equal) {
		return [value](bool b) { return b == value; };
	}
	return [value](bool b) { return b != value; };
}
};
template<>
struct Check<int> {
static delegate<bool(int)> getCheck(TransitionImage const& t) {
	// ASSERT if wrong type

	int value;
	std::istringstream (t.compareValue) >> value;
	if (t.compareSymbol == TransitionImage::Compare::Equal) {
		return [value](int i) { return i == value; };
	} else if (t.compareSymbol == TransitionImage::Compare::Greater) {
		return [value](int i) { return i > value; };
	} else if (t.compareSymbol == TransitionImage::Compare::GreaterEqual) {
		return [value](int i) { return i >= value; };
	} else if (t.compareSymbol == TransitionImage::Compare::Less) {
		return [value](int i) { return i < value; };
	} else if (t.compareSymbol == TransitionImage::Compare::LessEqual) {
		return [value](int i) { return i <= value; };
	}
	return [value](int i) { return i != value; };
}
};
template<>
struct Check<double> {
static delegate<bool(double)> getCheck(TransitionImage const& t) {
	// ASSERT if wrong type

	double value;
	std::istringstream (t.compareValue) >> value;
	if (t.compareSymbol == TransitionImage::Compare::Equal) {
		return [value](double f) { return f == value; };
	} else if (t.compareSymbol == TransitionImage::Compare::Greater) {
		return [value](double f) { return f > value; };
	} else if (t.compareSymbol == TransitionImage::Compare::GreaterEqual) {
		return [value](double f) { return f >= value; };
	} else if (t.compareSymbol == TransitionImage::Compare::Less) {
		return [value](double f) { return f < value; };
	} else if (t.compareSymbol == TransitionImage::Compare::LessEqual) {
		return [value](double f) { return f <= value; };
	}
	return [value](double f) { return f != value; };
}
};

template<>
struct Check<std::string> {
static delegate<bool(std::string)> getCheck(TransitionImage const& t) {
	// ASSERT if wrong type
	std::string value = t.compareValue;
	if (t.compareSymbol == TransitionImage::Compare::Equal) {
		return [value](std::string s) { return s == value; };
	}
	return [value](std::string s) { return s != value; };
}
};

template <typename R, typename... Parameters>
inline bool Universe::reg_condition(std::string _name, delegate<R(std::tuple<Parameters...>)> _function) {
	if (!_function) return false;
	conditionParaMap[_name] = [_function](ParameterImageList _list, TransitionImage const* transition) {
//!TODO ASSERT needed if wrong number of parameters is given
		delegate<bool(R)> check = Check<R>::getCheck(*transition);
		delegate<R()>     part = partialBind(_function, _list);
		delegate<bool()> f = [=]() { return check(part());};

		return f;
	};
	return true;
}



}

#endif
