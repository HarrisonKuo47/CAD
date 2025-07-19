#PA2 cpp compiler
.PHONY: all run clean

all:
	g++ -std=c++11 -O2 PA2.cpp -o PA2

run:
	./PA2 $(input) $(output)

clean:
	rm -f test *.o *.out
 
 