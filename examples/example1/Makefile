all: example

example: example.cpp example.sm
	../../ssm.pl -i '"../../include/ssm.h"' --cpp11 example.sm
	g++ -std=c++11 -o example example.cpp

run: example
	./example

clean:
	rm -f example
	rm -f example.sm.h
