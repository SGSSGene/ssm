#include "Task.h"

bool Task::isFinished() const {
	return i == finish;
}
void Task::inc() {
	i = i+1;
}
