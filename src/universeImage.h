#ifndef SIMPLESTATEMACHINE_UNIVERSEIMAGE_H
#define SIMPLESTATEMACHINE_UNIVERSEIMAGE_H

#include "delegate.h"

#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace SimpleStateMachine {

struct ParameterImage {
	enum Type { Bool, Integer, String };
	Type type;

	bool v_b;
	int v_i;
	std::string v_s;

	template<class T> T getValue() const;
};
template<>
inline bool ParameterImage::getValue() const {
	return v_b;
}
template<>
inline int ParameterImage::getValue() const {
	return v_i;
}
template<>
inline std::string ParameterImage::getValue() const {
	return v_s;
}


struct ParameterImageList : public std::vector<ParameterImage> {};

struct FunctionImage {
	std::string decoratedFunction;
	std::string genericFunction;
	std::string signature;
	ParameterImageList parameters;
};

struct TransitionImage {
	FunctionImage functionImage;
	std::string targetState;

	delegate<bool(bool)> check_b;
	delegate<bool(int)> check_i;
	delegate<bool(std::string)> check_s;

	template<class T> delegate<bool(T)> getCheck() const;
};
template<>
inline auto TransitionImage::getCheck() const
	-> decltype(check_b) {
	return check_b;
}
template<>
inline auto TransitionImage::getCheck() const
	-> decltype(check_i) {
	return check_i;
}
template<>
inline auto TransitionImage::getCheck() const
	-> decltype(check_s) {
	return check_s;
}

struct StateImage {
	FunctionImage function;

	bool        force;
	std::vector<TransitionImage> transitionSet;
	std::vector<std::string>     machineNameSet;
};
struct MachineImage {
	std::string initialState;
	std::map<std::string, StateImage> stateImageMap;
};
typedef std::map<std::string, MachineImage> UniverseImage;

}
#endif
