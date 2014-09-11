#ifndef SIMPLESTATEMACHINE_CONDITION_H
#define SIMPLESTATEMACHINE_CONDITION_H

#include "delegate.h"
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace SimpleStateMachine {

class ParameterImageList;
class TransitionImage;

typedef delegate<bool()> Condition;
typedef std::map<std::string, Condition> ConditionMap;

typedef delegate<delegate<bool()>(ParameterImageList, TransitionImage const*)> ConditionPara;
typedef std::map<std::string, ConditionPara>                     ConditionParaMap;
}

#endif
