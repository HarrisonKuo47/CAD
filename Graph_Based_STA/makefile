#PA2 cpp compiler

.PHONY: all run clean source

all: 
	@g++ -std=c++11 main.cpp ./src/lib_Parse.cpp ./src/Graph.cpp -o STA
run: 
	@./STA $(input) test_lib.lib
clean: 
	@rm -f *.o
	@rm -f STA
	@rm -f *.txt
source:
	@source /usr/cad/cadence/CIC/incisiv.cshrc
	@source /usr/cad/synopsys/CIC/verdi.cshrc


