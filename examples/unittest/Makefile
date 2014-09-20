all: unittest

unittest: unittest.cpp unittest.sm
	../../ssm.pl -i '"../../include/ssm.h"' --cpp11 unittest.sm
	g++ -std=c++11 -O3 \
	    -o unittest \
	    unittest.cpp \

run: unittest
	./unittest

clean:
	rm -f unittest
	rm -f unittest.sm.h

