all: unittest

%.sm.h: %.sm
	../../ssm.pl -i '"../../include/ssm.h"' --cpp11 $?

unittest: unittest.cpp $(patsubst %,%.h,$(wildcard *.sm))
	g++ -std=c++11 -O3 \
	    -o unittest \
	    unittest.cpp \

run: unittest
	./unittest

clean:
	rm -f unittest
	rm -f *.sm.h

