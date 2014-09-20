#include <iostream> // Includes std::cout

#include "example.sm.h" // Includes our state machine


class MyClass {
public:
	void helloWorld() {
		std::cout<<"Hello World"<<std::endl;
	}
};

int main(int argc, char** args) {
	// Create instacne of MyClass
	MyClass myClass;

	// Create instance of our state machine. Example1 is provided by example.sm.h
	// As an argument it wants the object that should be used by the state machine
	MyStateMachine machine(&myClass);

	// run the state machine
	machine.run();
	return 0;
}
