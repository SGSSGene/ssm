all: ssm ssm-lib

ssm: src/ssm/* ssm-lib
	g++ -std=c++11 \
	    -o ssm \
	    -Isrc/ \
	    -Wall \
	    src/ssm/main.cpp \
	    libssm.so


ssm-lib: src/ssm/*
	g++ -std=c++11 \
	    -fPIC\
	    -shared \
	    -Wall \
	    -Isrc/ \
	    src/ssm/machine.cpp \
	    src/ssm/state.cpp \
	    src/ssm/universe.cpp \
	    src/ssm/parser.cpp \
	    -o libssm.so

install: all
	cp ssm /usr/local/bin
	cp libssm.so /usr/lib
	mkdir -p /usr/local/include/ssm
	cp src/ssm/action.h \
	   src/ssm/autoRegister.h \
	   src/ssm/condition.h \
	   src/ssm/delegate.h \
	   src/ssm/helper.h \
	   src/ssm/machine.h \
	   src/ssm/simpleStateMachine.h \
	   src/ssm/state.h \
	   src/ssm/transition.h \
	   src/ssm/universe.h \
	   src/ssm/universeImage.h \
	   /usr/local/include/ssm

clean:
	rm -f ssm
	rm -f libssm.a

