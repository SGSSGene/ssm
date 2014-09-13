all: unittest1 unittest2 example1

unittest1.o: unittest1.cpp
	g++ -c -std=c++11 -O3 unittest1.cpp

unittest1: unittest1.o
	g++ -O3 \
	    -o unittest1 \
	    unittest1.o \
	    -lssm

unittest2: ../src/* unittest2.cpp unittest1.sm
	ssm --c++11 unittest1
	g++ -std=c++11 \
	    -O3 \
	    -o unittest2 \
	    unittest2.cpp \
	    -lssm

example1: ../src/* example1.cpp example1.sm
	ssm --c++11 example1
	g++ -std=c++11 \
	    -O2 \
		-o example1 \
	    example1.cpp \
	    -lssm

clean:
	rm -f unittest1
	rm -f unittest2
	rm -f unittest1.sm.h
	rm -f unittest2.sm.h
	rm -f example1
	rm -f example1.sm.h
