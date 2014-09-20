#ifndef SIMPLESTATEMACHINE_TIMER
#define SIMPLESTATEMACHINE_TIMER

#include <chrono>
#include <iostream>
class Timer {
private:
	std::chrono::time_point<std::chrono::system_clock> start;
public:
	void startTimer() {
		start = std::chrono::system_clock::now();
	}
	double getTime() const {
		std::chrono::time_point<std::chrono::system_clock> current = std::chrono::system_clock::now();
		int elapsed_milliseconds  = std::chrono::duration_cast<std::chrono::milliseconds>(current-start).count();

		return double(elapsed_milliseconds) * 0.001;
	}
};

#endif
