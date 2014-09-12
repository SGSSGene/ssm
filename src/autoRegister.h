#ifndef SIMPLESTATEMACHINE_AUTOREGISTER_H
#define SIMPLESTATEMACHINE_AUTOREGISTER_H

#include <iostream>
#include <functional>
#include <tuple>

#define DEF_AUTO_REGISTER_BEGIN \
namespace SimpleStateMachine { \
template <class O, typename ...Args> \
void _autoRegister(Universe* universe, O* o) {

#define DEF_AUTO_REGISTER_ACTION(NAME) { \
	auto fct = get_method_call_##NAME(o); \
	universe->reg_action(#NAME, fct); \
}

#define DEF_AUTO_REGISTER_CONDITION(NAME) { \
	auto fct = get_method_call_##NAME(o); \
	universe->reg_condition(#NAME, fct); \
}


#define DEF_AUTO_REGISTER_END }}

#define DEF_GET_METHOD_CALL(NAME, CALL, ...) \
namespace SimpleStateMachine { \
using namespace std; \
template <class T, typename R, typename ...Args> \
auto get_method_call_##NAME##_impl(T* t, int, std::tuple<Args...> p = std::tuple<Args...>()) \
	-> delegate<decltype(t->CALL)(std::tuple<Args...>)> { \
	return [t](std::tuple<Args...> const& p) { \
		return t->CALL; \
	}; \
} \
template <class T, typename R, typename ...Args> \
delegate<R(std::tuple<Args...>)> get_method_call_##NAME##_impl(T* t, long, std::tuple<Args...> p = std::tuple<Args...>()) { \
	return delegate<R(std::tuple<Args...>)>(); \
} \
template <class T> \
auto get_method_call_##NAME(T* t) \
	-> decltype(get_method_call_##NAME##_impl<T, __VA_ARGS__>(t, 0)) { \
	return get_method_call_##NAME##_impl<T, __VA_ARGS__>(t, 0); \
} }

#endif
