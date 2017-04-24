INC=-I./include -I../include
FLG=-std=c++11 -pthread
LIB=lib/gtest_main.a lib/libbenchmark.a
SRC=./src
BIN=./bin

all: all_tests all_benchmarks

all_tests:
	g++ $(SRC)/*_test.cpp $(LIB) $(INC) $(FLG) -o $(BIN)/$@

all_benchmarks:
	g++ $(SRC)/*_benchmark.cpp $(LIB) $(INC) $(FLG) -o $(BIN)/$@

%_test:
	g++ $(SRC)/$@.cpp $(LIB) $(INC) $(FLG) -o $(BIN)/$@

%_benchmark:
	g++ $(SRC)/$@.cpp $(LIB) $(INC) $(FLG) -o $(BIN)/$@

clean:
	find -name '*~' -delete
	find -name '*.sw*' -delete
