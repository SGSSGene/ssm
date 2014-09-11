#include "magic.h"
void Magic::inc() {
	i = i + 1;
}
std::function<void()> Magic::getAction() const {
	return [this]() {
		inc();
	};
}
std::function<bool()> Magic::getCondition() const {
	return [this]() {
		return i >= 10000000;
	};
}



