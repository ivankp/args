CXXFLAGS := -std=c++14 -Wall -pedantic -Iinclude -O2 -g -fmax-errors=3

NODEPS := clean
.PHONY: all clean

all: test/test

test/test: test/test.cc src/parse_args.cc
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	@rm -fv test/test
