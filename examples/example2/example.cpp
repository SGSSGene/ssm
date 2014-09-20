#include "example.sm.h"

class Module {
public:
	bool condition() {
		return false;
	}
	void message(std::string s) {
		std::cout<<s<<std::endl;
	}
};


int main(int, char**) {
	Module module;

	MyStateMachine machine(&module);
	machine.run();

	return 0;
}
