CXXFLAGS := -std=c++14 -Wall -O2 -g -Iinclude -fmax-errors=3
# CXXFLAGS := -std=c++14 -Wall -O3 -Iinclude -flto -funroll-loops

NODEPS := clean
.PHONY: all clean

all: test/test

HH := $(wildcard include/*.hh)

test/args_parser.o: src/args_parser.cc $(HH)
	$(CXX) $(CXXFLAGS) -c $(filter %.cc,$^) -o $@

test/test.o: test/test.cc $(HH)
	$(CXX) $(CXXFLAGS) -c $(filter %.cc,$^) -o $@

test/test: test/test.o test/args_parser.o $(HH)
	$(CXX) $(CXXFLAGS) $(filter %.o,$^) -o $@

clean:
	@rm -fv test/args_parser.o test/test.o test/test
