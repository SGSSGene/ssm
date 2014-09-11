#ifndef SIMPLESTATEMACHINE_UNIVERSE_H
#define SIMPLESTATEMACHINE_UNIVERSE_H


#include "state.h"
#include "transition.h"
#include "machine.h"
#include "universeImage.h"

#include <sstream>
#include <iostream>

namespace SimpleStateMachine {

template <typename T>
struct identity
{
  typedef T type;
};

class Universe {
private:
	std::stringstream errorMessages;

	std::map<std::string, std::string> requiredConditions;
	std::map<std::string, std::string> requiredActions;

	ActionParaMap    actionParaMap;
	ConditionParaMap conditionParaMap;
public:
	Universe();
	Universe(std::istream& _istream, std::ostream& _ostream);
	Universe(std::istream& _istream);

	template <typename... Parameters>
	inline bool reg_action(std::string _name, delegate<void(std::tuple<Parameters...>)> _function);

	template <typename R, typename... Parameters>
	inline bool reg_condition(std::string _name, delegate<R(std::tuple<Parameters...>)> _function);

	std::string getErrorMessages() const { return errorMessages.str(); }

	void appendError(int _lineNbr, std::string _msg, std::string _line);

	Universe const& operator=(Universe const& _universe);
	Universe const& operator=(Universe&& _universe);

	std::map<std::string, std::string> const& getRequiredConditions() const { return requiredConditions; }
	std::map<std::string, std::string> const& getRequiredActions()    const { return requiredActions; }

	UniverseImage parse(std::istream& _ifile);
	std::shared_ptr<Machine> bootstrap(std::string const& _ifile);
	std::shared_ptr<Machine> bootstrap(std::istream& _ifile);
private:
	std::shared_ptr<Machine> bootstrap(UniverseImage const& _universeImage, ActionParaMap _actionParaMap, ConditionParaMap _conditionParaMap, std::string const& _machineName);

	void extractAllRequirements(UniverseImage const& universeImage);
};

template <typename... Args>
struct PartialBind_Impl;

template <typename R, typename First, typename ...Args>
struct PartialBind_Impl<R, First, Args...> {
	static std::tuple<First, Args...> partialBind_impl(delegate<R(std::tuple<First, Args...>)> _f, ParameterImageList* parameters) {

		auto value = parameters->at(0).getValue<First>();
		parameters->erase(parameters->begin());

		auto f = delegate<R(std::tuple<Args...>)>();
		auto tuple_parameters = PartialBind_Impl<R, Args...>::partialBind_impl(f, parameters);
		auto tuple_parameters_full = std::tuple_cat(std::make_tuple(value), tuple_parameters);
		return tuple_parameters_full;
	}
};
template <typename R, typename First>
struct PartialBind_Impl<R, First> {
	static std::tuple<First> partialBind_impl(delegate<R(std::tuple<First>)> _f, ParameterImageList* parameters) {
		auto value = parameters->at(0).getValue<First>();
		std::tuple<First> tuple_parameters = std::make_tuple(value);
		parameters->erase(parameters->begin());

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
template <typename R, typename... Parameters>
inline bool Universe::reg_condition(std::string _name, delegate<R(std::tuple<Parameters...>)> _function) {
	if (!_function) return false;
	conditionParaMap[_name] = [_function](ParameterImageList _list, TransitionImage const* transition) {
//!TODO ASSERT needed if wrong number of parameters is given
		delegate<bool(R)> check = transition->getCheck<R>();
		delegate<R()>     part = partialBind(_function, _list);
		delegate<bool()> f = [=]() { return check(part());};

		return f;
	};
	return true;
}



}

#endif
