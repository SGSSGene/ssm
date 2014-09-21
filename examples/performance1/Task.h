#ifndef TASK
#define TASK

class Task {
private:
	int i;
	int finish;
public:
	Task(int _finish) :i(0), finish(_finish) {}

	bool isFinished() const;
	void inc();
};
#endif
