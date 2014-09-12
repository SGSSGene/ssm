all: unittest1 unittest2 example1

unittest1: ../src/* unittest1.cpp
	g++ -std=c++11 \
	    -O2 \
	    -o unittest1 \
	    -I ../src/ \
	    ../src/machine.cpp \
	    ../src/state.cpp \
	    ../src/universe.cpp \
	    unittest1.cpp

unittest2: ../src/* unittest2.cpp unittest1.sm
	ssm --c++11 unittest1 ../src
	g++ -std=c++11 \
	    -O2 \
	    -o unittest2 \
	    -I ../src/ \
	    ../src/machine.cpp \
	    ../src/state.cpp \
	    ../src/universe.cpp \
	    unittest2.cpp

example1: ../src/* example1.cpp example1.sm
	ssm --c++11 example1 ../src
	g++ -std=c++11 \
	    -O2 \
		-o example1 \
		-I ../src/ \
	    ../src/machine.cpp \
	    ../src/state.cpp \
	    ../src/universe.cpp \
	    example1.cpp

clean:
	rm -f unittest1
	rm -f unittest2
	rm -f unittest1.sm.h
	rm -f unittest2.sm.h
	rm -f example1
	rm -f example1.sm.h
