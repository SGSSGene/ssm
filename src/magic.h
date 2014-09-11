#pragma once

#include <functional>

struct Magic {
	Magic() : i(0) {}
	int i;
	void inc();

	std::function<bool()> getCondition() const;
	std::function<void()> getAction() const;
};

