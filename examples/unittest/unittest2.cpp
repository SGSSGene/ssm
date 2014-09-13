#include "unittest1.sm.h"

#include <cmath>
#include <chrono>

class GeneralTest {
public:
	void message(std::string message) {
		std::cout<<message<<std::endl;
	}
};

class BooleanTest {
public:
	bool _mirror;
	BooleanTest() : _mirror(false) {}
	BooleanTest(bool b) : _mirror(b) {}
	bool condition_true() { return (!_mirror)?true:false; }
	bool condition_false() { return (!_mirror)?false:true; }
	bool identity(bool b) { return (!_mirror)?b:!b; }
	bool negate(bool b) { return (!_mirror)?!b:b; }
	BooleanTest mirror(bool b) { return BooleanTest(b); }
	bool equals(bool b1, bool b2) { return (!_mirror)?b1 == b2:b1 != b2; }
};
class IntegerTest {
public:
	int getConst1() { return 1;	}
	int getConst2() { return 2; }
	bool is1(int i) { return i == 1; }
	int keepSign(int i, bool b) { return b?i:std::abs(i); }
};
class FloatTest {
public:
	double getConst1f() { return 1.;	}
	double getConst2f() { return 2.; }
	bool is1f(double f) { return f == 1.; }
		double keepSignf(double f, bool b) { return b?f:std::abs(f); }
};

class StringTest {
public:
	std::string getConstString() { return "test"; }
	bool isStringTest(std::string s) { return s == "test"; }
	std::string copyString(std::string s) { return s; }
};

int main(int, char**) {
	GeneralTest c1;
	BooleanTest c2;
	IntegerTest c3;
	FloatTest   c4;
	StringTest  c5;

	Unittest1 machine(std::make_tuple(&c1, &c2, &c3, &c4, &c5));

	machine.run();

	return 0;
}
