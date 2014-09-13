#ifndef SIMPLESTATEMACHINE_UNIVERSEIMAGE_H
#define SIMPLESTATEMACHINE_UNIVERSEIMAGE_H

#include "delegate.h"

#include <map>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <vector>

namespace SimpleStateMachine {


struct ParameterImage {
	bool v_b;
	int v_i;
	double v_f;
	std::string v_s;

	void stream(std::ostream& os, std::string tabs) const {
		os<<tabs<<"{ "<<(v_b?"true":"false")<<", "<<v_i<<", "<<v_f<<", std::string(\""<<v_s<<"\") }";
	}

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
inline double ParameterImage::getValue() const {
	return v_f;
}
template<>
inline std::string ParameterImage::getValue() const {
	return v_s;
}


struct ParameterImageList {
	std::vector<ParameterImage> pList;
	void stream(std::ostream& os, std::string tabs) const {
		os<<tabs<<"{ {// ParameterImageList\n";
		for (auto s : pList) {
			s.stream(os, tabs+"\t");
			os<<",\n";
		}
		os<<tabs<<"} }";
	}
};

struct FunctionImage {
	std::string decoratedFunction;
	std::string genericFunction;
	std::string signature;
	std::string extendedFunction;
	std::string broadSignature;
	ParameterImageList parameters;
	void stream(std::ostream& os, std::string tabs) const {
		std::string tab2 = tabs+"\t";
		os<<tabs<<"{ // FunctionImage\n";
		os<<tab2<<"std::string(\""<<decoratedFunction<<"\"), std::string(\""<<genericFunction<<"\"), std::string(\""<<signature<<"\"), std::string(\"\"), std::string(\"\"),\n";
		parameters.stream(os, tab2); os<<"\n";
		os<<tabs<<"}";
	}
};

struct TransitionImage {
	FunctionImage functionImage;
	std::string targetState;

	enum DataType { Boolean, Integer, Float, String };
	DataType dataType;

	enum class Compare { Equal, Unequal, LessEqual, GreaterEqual, Less, Greater };
	Compare compareSymbol;

	std::string compareValue;

	void stream(std::ostream& os, std::string tabs) const {
		std::string tab2 = tabs+"\t";
		os<<tabs<<"{ // Transition Image\n";
		functionImage.stream(os, tab2); os<<",\n";
		os<<tab2<<"std::string(\""<<targetState<<"\"), ";
		if (dataType == DataType::Boolean) os<<"SimpleStateMachine::TransitionImage::DataType::Boolean";
		else if (dataType == DataType::Integer) os<<"SimpleStateMachine::TransitionImage::DataType::Integer";
		else if (dataType == DataType::Float) os<<"SimpleStateMachine::TransitionImage::DataType::Float";
		else if (dataType == DataType::String) os<<"SimpleStateMachine::TransitionImage::DataType::String";
		os<<", ";
		if (compareSymbol == Compare::Equal) os<<"SimpleStateMachine::TransitionImage::Compare::Equal";
		else if (compareSymbol == Compare::Unequal) os<<"SimpleStateMachine::TransitionImage::Compare::Unequal";
		else if (compareSymbol == Compare::LessEqual) os<<"SimpleStateMachine::TransitionImage::Compare::LessEqual";
		else if (compareSymbol == Compare::GreaterEqual) os<<"SimpleStateMachine::TransitionImage::Compare::GreaterEqual";
		else if (compareSymbol == Compare::Less) os<<"SimpleStateMachine::TransitionImage::Compare::Less";
		else if (compareSymbol == Compare::Greater) os<<"SimpleStateMachine::TransitionImage::Compare::Greater";
		os<<", ";
		os<<"std::string(\""<<compareValue<<"\")\n";
		os<<tabs<<"}";
	}
};

struct StateImage {
	FunctionImage function;

	enum class ExecuteType { Any, Once, MaxOnce, MinOnce };
	ExecuteType executeType;

	std::vector<TransitionImage> transitionSet;
	std::vector<std::string>     machineNameSet;
	void stream(std::ostream& os, std::string tabs) const {
		std::string tab2 = tabs+"\t";
		os<<tabs<<"{ // State Image\n";
		function.stream(os, tab2); os <<",\n";

		if (executeType == ExecuteType::Any) {
			os<<tab2<<"SimpleStateMachine::StateImage::ExecuteType::Any"<<",\n";
		} else if (executeType == ExecuteType::Once) {
			os<<tab2<<"SimpleStateMachine::StateImage::ExecuteType::Once"<<",\n";
		} else if (executeType == ExecuteType::MaxOnce) {
			os<<tab2<<"SimpleStateMachine::StateImage::ExecuteType::MaxOnce"<<",\n";
		} else if (executeType == ExecuteType::MinOnce) {
			os<<tab2<<"SimpleStateMachine::StateImage::ExecuteType::MinOnce"<<",\n";
		}

		os<<tab2<<"{ // TransitionList\n";
		for (auto t : transitionSet) {
			t.stream(os, tab2+"\t"); os<<",\n";
		}
		os<<tab2<<"},\n";
		os<<tab2<<"{ // Sub MachineList\n";
		for (auto s : machineNameSet) {
			os<<tab2<<"\t"<<"std::string(\""<<s<<"\"),\n";
		}
		os<<tab2<<"}\n";
		os<<tabs<<"}";
	}
};
struct MachineImage {
	std::string initialState;
	std::map<std::string, StateImage> stateImageMap;

	void stream(std::ostream& os, std::string tabs) const {
		std::string tab2 = tabs+"\t";
		std::string tab3 = tab2+"\t";
		os<<tabs<<"{ // Machine Image\n";
		os<<tab2<<"std::string(\""<<initialState<<"\"),\n";
		os<<tab2<<"{ // StateImageMap\n";
		for (auto e : stateImageMap) {
			os<<tab3<<"{ std::string(\""<<e.first<<"\"),\n";
			e.second.stream(os, tab3+"\t"); os<<"\n";
			os<<tab3<<"},\n";
		}
		os<<tab2<<"}\n";
		os<<tabs<<"}";
	}
};
typedef std::map<std::string, MachineImage> UniverseImage;

}
#endif
