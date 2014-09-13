#include <iostream> // Includes std::cout

#include "example1.sm.h" // Includes our state machine


class MyClass {
public:
	void helloWorld() {
		std::cout<<"Hello World"<<std::endl;
	}
};

int main(int argc, char** args) {
	// Create instacne of MyClass
	MyClass myClass;

	// Create tuple of all objects that we want our state machine to use
	auto objects = std::make_tuple(&myClass);

	// Create instance of our state machine. Example1 is provided by example1.sm.h
	// As an argument it wants a tuple with objects that should be used by the state machine
	Example1 example1(objects);

	// run the state machine
	example1.run();
	return 0;
}
