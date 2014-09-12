#include "simpleStateMachine.h"

#include <chrono>

DEF_GET_METHOD_CALL(message, message(get<0>(p)), void, std::string);
DEF_GET_METHOD_CALL(condition_true, condition_true(), bool);
DEF_GET_METHOD_CALL(condition_false, condition_false(), bool);
DEF_GET_METHOD_CALL(identity, identity(get<0>(p)), bool, bool);
DEF_GET_METHOD_CALL(negate, negate(get<0>(p)), bool, bool);
DEF_GET_METHOD_CALL(mirror_condition_true, mirror(get<0>(p)).condition_true(), bool, bool);
DEF_GET_METHOD_CALL(mirror_condition_false, mirror(get<0>(p)).condition_false(), bool, bool);
DEF_GET_METHOD_CALL(mirror_identity, mirror(get<0>(p)).identity(get<1>(p)), bool, bool, bool);
DEF_GET_METHOD_CALL(mirror_negate, mirror(get<0>(p)).negate(get<1>(p)), bool, bool, bool);
DEF_GET_METHOD_CALL(equals, equals(get<0>(p), get<1>(p)), bool, bool, bool);
DEF_GET_METHOD_CALL(getConst1, getConst1(), int);
DEF_GET_METHOD_CALL(getConst2, getConst2(), int);
DEF_GET_METHOD_CALL(is1, is1(std::get<0>(p)), bool, int);
DEF_GET_METHOD_CALL(keepSign, keepSign(get<0>(p), get<1>(p)), int, int, bool);
DEF_GET_METHOD_CALL(getConstString, getConstString(), std::string);
DEF_GET_METHOD_CALL(isStringTest, isStringTest(get<0>(p)), bool, std::string);
DEF_GET_METHOD_CALL(copyString,   copyString(get<0>(p)), std::string, std::string);
DEF_GET_METHOD_CALL(i, i, int);
DEF_GET_METHOD_CALL(inc, inc(), void);
DEF_AUTO_REGISTER_BEGIN
	DEF_AUTO_REGISTER_ACTION(message)
	DEF_AUTO_REGISTER_ACTION(inc)
	DEF_AUTO_REGISTER_CONDITION(condition_true)
	DEF_AUTO_REGISTER_CONDITION(condition_false)
	DEF_AUTO_REGISTER_CONDITION(identity)
	DEF_AUTO_REGISTER_CONDITION(negate)
	DEF_AUTO_REGISTER_CONDITION(mirror_condition_true)
	DEF_AUTO_REGISTER_CONDITION(mirror_condition_false)
	DEF_AUTO_REGISTER_CONDITION(mirror_identity)
	DEF_AUTO_REGISTER_CONDITION(mirror_negate)
	DEF_AUTO_REGISTER_CONDITION(equals)
	DEF_AUTO_REGISTER_CONDITION(getConst1)
	DEF_AUTO_REGISTER_CONDITION(getConst2)
	DEF_AUTO_REGISTER_CONDITION(is1)
	DEF_AUTO_REGISTER_CONDITION(keepSign)
	DEF_AUTO_REGISTER_CONDITION(getConstString)
	DEF_AUTO_REGISTER_CONDITION(isStringTest)
	DEF_AUTO_REGISTER_CONDITION(copyString)
	DEF_AUTO_REGISTER_CONDITION(i)
DEF_AUTO_REGISTER_END

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
class StringTest {
public:
	std::string getConstString() { return "test"; }
	bool isStringTest(std::string s) { return s == "test"; }
	std::string copyString(std::string s) { return s; }
};

int main(int, char**) {
	SimpleStateMachine::Universe uni;

	GeneralTest c1;
	BooleanTest c2;
	IntegerTest c3;
	StringTest  c4;
	uni.autoRegister(&c1);
	uni.autoRegister(&c2);
	uni.autoRegister(&c3);
	uni.autoRegister(&c4);

	auto machine = uni.bootstrap("unittest1.sm");
	std::cout<<uni.getErrorMessages();

	std::cout<<std::boolalpha;

	while (machine->executeStep());

	return 0;
}
