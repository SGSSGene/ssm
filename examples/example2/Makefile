all: example2

example2: example2.cpp example2.sm
	ssm --c++11 example2.sm
	g++ -std=c++11 \
	    -o example2 \
	    unittest2.cpp \
	    -lssm

clean:
	rm -f example2
	rm -f example2.sm.h

