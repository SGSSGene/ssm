#include "Task.h"

bool Task::isFinished() {
	return i == finish;
}
void Task::inc() {
	i = i+1;
}
