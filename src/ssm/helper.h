#ifndef SIMPLESTATEMACHINE_HELPER
#define SIMPLESTATEMACHINE_HELPER

#include <ssm/universe.h>

namespace SimpleStateMachine {

template<int... Is>
struct seq {};

template<int N, int... Is>
struct gen_seq : gen_seq<N-1, N-1, Is...> {};

template<int... Is>
struct gen_seq<0, Is...> : seq<Is...> {};

template<typename T, typename F, int... Is>
void for_each_impl(T&& t, F f, seq<Is...>) {
	auto l = { (f(std::get<Is>(t)), 0)...};
}

template<typename... Ts, typename F>
void for_each_in_tuple(std::tuple<Ts...> const& t, F f) {
	for_each_impl(t, f, gen_seq<sizeof...(Ts)>());
}

struct RegAtUniverse {
	Universe* uni;
	RegAtUniverse(Universe* _uni) : uni(_uni) {}
	template<typename T>
	void operator () (T&& t) {
		uni->autoRegister(t);
	}
};
template<typename ...Para>
void autoRegisterAll(Universe* _uni, std::tuple<Para...> _p) {
	RegAtUniverse regAtUni(_uni);
	for_each_in_tuple(_p, regAtUni);
}



}
#endif
