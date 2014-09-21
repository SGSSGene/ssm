#include <iostream>

#include "example.sm.h"
#include "Task.h"

int main(int argc, char** args) {
	Task taskForSSM(100000000);
	Task taskForRaw(100000000);
	TaskMachine machine(&taskForSSM);

	std::chrono::time_point<std::chrono::system_clock> start_SSM = std::chrono::system_clock::now();
	machine.run();
	std::chrono::time_point<std::chrono::system_clock> stop_SSM = std::chrono::system_clock::now();

	std::chrono::time_point<std::chrono::system_clock> start_Raw = std::chrono::system_clock::now();
	while(!taskForRaw.isFinished()) taskForRaw.inc();
	std::chrono::time_point<std::chrono::system_clock> stop_Raw = std::chrono::system_clock::now();

	int ssm_time  = std::chrono::duration_cast<std::chrono::milliseconds>(stop_SSM - start_SSM).count();
	int raw_time  = std::chrono::duration_cast<std::chrono::milliseconds>(stop_Raw - start_Raw).count();


	std::cout<<"SSM: "<<ssm_time<<"ms"<<std::endl;
	std::cout<<"Raw: "<<raw_time<<"ms"<<std::endl;

	return 0;
}
