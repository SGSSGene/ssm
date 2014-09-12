all: src/*
	g++ -std=c++11 \
	    -o ssm \
	    src/machine.cpp \
	    src/state.cpp \
	    src/universe.cpp \
	    src/main.cpp

install: all
	cp ssm /usr/local/bin

clean:
	rm -f ssm

