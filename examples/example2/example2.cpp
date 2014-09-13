#include "example2.sm.h"

class Module1 {
public:
	bool condition() {
		return false;
	}
	void message(std::string s) {
		std::cout<<s<<std::endl;
	}
};


int main(int, char**) {
	Module1 m1;

	Example1 machine1(std::make_tuple(&m1));
	machine1.run();


	return 0;
}
