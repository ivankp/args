CXXFLAGS := -std=c++14 -Wall -O2 -g -Iinclude -fmax-errors=3

NODEPS := clean
.PHONY: all clean

all: test/test

test/test: test/test.cc src/parse_args.cc include/parse_args.hh
	$(CXX) $(CXXFLAGS) $(filter %.cc,$^) -o $@

clean:
	@rm -fv test/test
