all: unittest

%.sm.h: %.sm
	../../ssm.pl -i '"../../include/ssm.h"' --cpp11 $?

unittest: unittest.cpp unittest.sm.h
	g++ -std=c++11 -O3 \
	    -o unittest \
	    unittest.cpp \
	    -Wfatal-errors \

run: unittest
	./unittest

clean:
	rm -f unittest
	rm -f *.sm.h

