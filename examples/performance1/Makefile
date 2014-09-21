all: example

example: example.cpp example.sm
	../../ssm.pl -i '"../../include/ssm.h"' --cpp11 example.sm
	g++ -O3 -std=c++11 -o example example.cpp Task.cpp

run: example
	./example

clean:
	rm -f example
	rm -f example.sm.h
