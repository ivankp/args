CXXFLAGS := -std=c++14 -Wall -O2 -g -Iinclude -fmax-errors=3

NODEPS := clean
.PHONY: all clean

all: test/test

test/test: test/test.cc src/args_parser.cc include/args_parser.hh
	$(CXX) $(CXXFLAGS) $(filter %.cc,$^) -o $@

clean:
	@rm -fv test/test
