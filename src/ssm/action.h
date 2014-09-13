#ifndef SIMPLESTATEMACHINE_ACTION_H
#define SIMPLESTATEMACHINE_ACTION_H

#include <ssm/delegate.h>
#include <functional>
#include <map>

namespace SimpleStateMachine {

class ParameterImageList;

typedef delegate<void()>              Action;
typedef std::map<std::string, Action> ActionMap;

typedef delegate<delegate<void()>(ParameterImageList)> ActionPara;
typedef std::map<std::string, ActionPara>              ActionParaMap;

}

#endif
