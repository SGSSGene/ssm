#ifndef SIMPLESTATEMACHINE_H
#define SIMPLESTATEMACHINE_H

#include <map>
#include <memory>
#include <set>
#include <tuple>
#include <vector>
#include <cassert>
#include <iostream>

// Module which can used by the state machine
#include "Timer.h"
#include "delegate.h"

namespace SimpleStateMachine {

class Parameter {
public:
	virtual ~Parameter() {}
};

template<typename ...Args>
class Parameter_Impl : public Parameter {
public:
	Parameter_Impl(std::tuple<Args...> _tuple) : tuple(_tuple) {}
	std::tuple<Args...> tuple;
};

typedef delegate<void()>                      Action;
typedef delegate<Action(Parameter const*)>    ActionPara;
typedef std::map<std::string, ActionPara>     ActionParaMap;

typedef delegate<bool()>                      Condition;
typedef delegate<Condition(Parameter const*)> ConditionPara;
typedef std::map<std::string, ConditionPara>  ConditionParaMap;


class State;
class Machine;
typedef std::vector<std::unique_ptr<Machine>>     MachinePtrList;
typedef std::vector<std::unique_ptr<State>>       StatePtrList;
typedef std::pair<Condition, State*>              Transition;
typedef std::vector<Transition>                   TransitionList;

class State {
public:
	enum class ExecuteType { Any, Once, MaxOnce, MinOnce };
protected:
	ExecuteType    executeType;
	Action         action;
	MachinePtrList machines;
	TransitionList transitions;
	bool           fired;

public:
	State(ExecuteType _executeType, Action _action, MachinePtrList& _machinePtrList)
		: executeType(_executeType)
		, action(_action)
		, machines(std::move(_machinePtrList)) {
	}
	void addTransition(Condition c, State* s) {
		transitions.push_back(std::make_pair(c, s));
	}
protected:
	inline void startMachines();
	inline void runMachines();

public:
	void enter() {
		startMachines();
		fired = false;
	}
	void doAction() {
		if (action) {
			if (!fired || executeType == ExecuteType::Any || executeType == ExecuteType::MinOnce) {
				action();
				fired = true;
			}
		}
		runMachines();
	}
	State* step() {
		// Check for transition
		if (fired || executeType == ExecuteType::Any || executeType == ExecuteType::MaxOnce) {
			for (auto& t : transitions) {
				if (t.first()) {
					t.second->enter();
					return t.second->step();
				}
			}
		}
		doAction();
		return this;
	}
	inline bool hasTransitions() const;
};

class Machine {
protected:
	State*       initState;
	State*       currentState;
public:
	Machine() {};
	void setInitState(State* _initState) { initState = _initState; }
	void start() {
		currentState = initState;
		currentState->enter();
		assert(currentState != nullptr);
	}
	void step() {
		assert(currentState != nullptr);
		currentState = currentState->step();
		assert(currentState != nullptr);
	}
	bool hasTransitions() const {
		return currentState->hasTransitions();
	}
	State const* getCurrentState() const { return currentState; }
};

inline void State::startMachines() {
	for (auto& m : machines) {
		m->start();
	}
}
inline void State::runMachines() {
	for (auto& m : machines) {
		m->step();
	}
}
inline bool State::hasTransitions() const {
	if (transitions.size() > 0) return true;
	for (auto const& m : machines) {
		if (m->hasTransitions()) return true;
	}
	return false;
}

#define DEF_AUTO_REGISTER_BEGIN \
namespace SimpleStateMachine { \
template <class O> \
void _autoRegister(ActionParaMap* actionMap, ConditionParaMap* conditionMap, O* o, std::set<std::string>& neededMethods) {

#define DEF_AUTO_REGISTER_ACTION(NAME) { \
	auto fct = MethodCall::get_method_call_##NAME(o); \
	if (fct) { \
		(*actionMap)[#NAME] = fct; \
		if (neededMethods.find(#NAME) != neededMethods.end()) neededMethods.erase(neededMethods.find(#NAME));\
	} \
}

#define DEF_AUTO_REGISTER_CONDITION(NAME) { \
	auto fct = MethodCall::get_method_call_##NAME(o); \
	if (fct) { \
		(*conditionMap)[#NAME] = fct; \
		if (neededMethods.find(#NAME) != neededMethods.end()) neededMethods.erase(neededMethods.find(#NAME));\
	} \
}

#define DEF_AUTO_REGISTER_END }}

#define DEF_GET_METHOD_CALL_BEGIN \
namespace SimpleStateMachine { \
using namespace std; \
struct MethodCall { \

#define DEF_GET_METHOD_CALL(NAME, CALL, ...) \
template <class T, typename R, typename ...Args> \
static auto get_method_call_##NAME##_impl(T* t, int, std::tuple<Args...> p) \
	-> delegate<delegate<decltype(t->CALL)()>(Parameter const*)> { \
	((void)p); \
	return [t](Parameter const* _ps) { \
		auto ps = dynamic_cast<Parameter_Impl<Args...> const*>(_ps); \
		std::tuple<Args...> p = ps->tuple; \
		return [t, p]() { return t->CALL; }; \
	}; \
} \
template <class T, typename R, typename ...Args> \
static delegate<delegate<R()>(Parameter const*)> get_method_call_##NAME##_impl(T* t, long, std::tuple<Args...> p) { \
	((void)t); \
	((void)p); \
	return delegate<delegate<R()>(Parameter const*)>(); \
} \
template <class T, typename R, typename ...Args> \
static auto _get_method_call_##NAME(T* t) \
	-> decltype(get_method_call_##NAME##_impl<T, R, Args...>(t, 0, std::tuple<Args...>())) { \
	return get_method_call_##NAME##_impl<T, R, Args...>(t, 0, std::tuple<Args...>()); \
} \
template <class T> \
static auto get_method_call_##NAME(T* t) \
	-> decltype(_get_method_call_##NAME<T, __VA_ARGS__>(t)) { \
	return _get_method_call_##NAME<T, __VA_ARGS__>(t); \
}

#define DEF_GET_METHOD_CALL_END \
};}

template<int... Is>
struct seq {};

template<int N, int... Is>
struct gen_seq : gen_seq<N-1, N-1, Is...> {};

template<int... Is>
struct gen_seq<0, Is...> : seq<Is...> {};

template<typename T, typename F, int... Is>
void for_each_impl(T && t, F f, seq<Is...>) {
	auto l = { (f(std::get<Is>(t)), 0)...};
	((void)l);
}

template<typename... Ts, typename F>
void for_each_in_tuple(std::tuple<Ts...> const& t, F f) {
	for_each_impl(t, f, gen_seq<sizeof...(Ts)>());
}

struct RegisterActionsAndConditions {
	ActionParaMap*    actionMap;
	ConditionParaMap* conditionMap;
	std::set<std::string>& neededMethods;

	RegisterActionsAndConditions(ActionParaMap* _actionMap, ConditionParaMap* _conditionMap, std::set<std::string>& _neededMethods)
		: actionMap(_actionMap), conditionMap(_conditionMap), neededMethods(_neededMethods) {}

	template <typename T>
	void operator () (T && t) {
		_autoRegister(actionMap, conditionMap, t, neededMethods);
	}
};
template<typename ...Para>
void autoRegisterAll(ActionParaMap* actionMap, ConditionParaMap* conditionMap, std::tuple<Para...> _p, std::set<std::string>& neededMethods) {
	RegisterActionsAndConditions _register(actionMap, conditionMap, neededMethods);
	// Adding default behavior
	(*conditionMap)["true_equal_p"]   = [](Parameter const* _p) {
		auto* p = dynamic_cast<Parameter_Impl<bool> const*>(_p);
		auto t = p->tuple;
		return [t] { return true == std::get<0>(t);  };
	};
	(*conditionMap)["true_unequal_p"]   = [](Parameter const* _p) {
		auto p = dynamic_cast<Parameter_Impl<bool> const*>(_p);
		auto t = p->tuple;
		return [t] { return true != std::get<0>(t);  };
	};
	(*conditionMap)["false_unequal_p"] = (*conditionMap)["true_equal_p"];
	(*conditionMap)["false_equal_p"]   = (*conditionMap)["true_unequal_p"];
	(*conditionMap)["else_equal_p"]    = (*conditionMap)["true_equal_p"];
	// Adding default actions
	(*actionMap)[""] = [](Parameter const* _p) { return []() {}; };
	for_each_in_tuple(_p, _register);
}
}

#endif
