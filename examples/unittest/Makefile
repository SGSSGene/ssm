all: unittest1 unittest2

unittest1: unittest1.cpp unittest1.sm
	g++ -std=c++11 -O3 \
	    -o unittest1 \
	    unittest1.cpp \
	    -lssm

unittest2: unittest2.cpp unittest1.sm
	ssm --c++11 unittest1
	g++ -std=c++11 -O3 \
	    -o unittest2 \
	    unittest2.cpp \
	    -lssm

run:
	./unittest1
	./unittest2

clean:
	rm -f unittest1
	rm -f unittest2
	rm -f unittest1.sm.h
	rm -f unittest2.sm.h

