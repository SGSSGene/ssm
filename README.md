# SSM - SimpleStateMachine

Tool to generate state machines for c++11.

## Features:
 0. Simple description language
 0. Super simple to use under C++11
 0. Using Graphviz dot to generate nice diagrams


### Description Language

A simple state machine: *example1.sm*
```
MyStateMachine
	MyStateStart: once helloWorld()
		true -> MyStateFinish
	MyStateFinish:
```
### Using it for C++11
Assuming you have following class.: 

```c
class MyClass {
public:
	void helloWorld() {
		std::cout<<"Hello World"<<std::endl;
	}
};
```
To use it with our state machine *example.sm*. We first run `ssm --c++11 example1.sm` this will create a single *example1.sm.h*

```C
	#include "example1.sm.h"
...
	MyClass myClass;

	Example1 example1(std::make_tuple(&myClass));
	example1.run();
```

Last step is compiling: `g++ -std=c++11 -o example1 example1.cpp -lssm`.
Done!
We can run *./example1* and get the output: `Hello World`.


### Using Graphviz
Running: `ssm --dot example1.sm | dot -Tpng > example1.png` creates:

![alt diagram](https://raw.githubusercontent.com/SGSSGene/ssm/master/github/images/example1.png)

## Getting started

Just download the library and run `make` and `make install`. It will install all needed header files and libssm.so  to the system.
```
git clone git@github.com:SGSSGene/ssm.git
cd ssm
make
sudo make install
```

