# SSM - SimpleStateMachine

Tool to generate state machines for c++11.

## Features:
 0. Simple description language
 0. Super simple to use under C++11
 0. Using Graphviz dot to generate nice diagrams


### Description Language

A simple state machine: *example1.sm*
```
export MyStateMachine

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
To use it with our state machine *example1.sm*. We first run `./ssm.pl --cpp11 example1.sm` this will create a single *example1.sm.h*

```C
	#include "example1.sm.h"
...
	MyClass myClass;

	MyStateMachine example1(&myClass);
	example1.run();
```

Last step is compiling: `g++ -std=c++11 -o example1 example1.cpp`.
Done!
We can run *./example1* and get the output: `Hello World`.


### Using Graphviz
Running: `./ssm.pl --png example1.sm` creates a file example1.sm.png:

![alt diagram](https://raw.githubusercontent.com/SGSSGene/ssm/master/github/images/example1.png)

You can also run `./ssm.pl --view example1.sm` which will display the diagram immediatly.

## Getting started

Just download everything from github and you are ready to start.
