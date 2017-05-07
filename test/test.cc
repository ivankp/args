#include <iostream>

#define test(var) \
  std::cout <<"\033[36m"<< #var <<"\033[0m"<< " = " << var << std::endl;

#include "parse_args.hh"

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char* argv[]) {
  int a, b, c;

  try {
    using namespace ivanp::args;
    parse_args()
    (&a,"-a","A")
    (&b,"--b-opt","B")
    (&c,'c',"C")
    .parse(argc,argv);
  } catch(std::exception& e) {
    cerr << e.what() << endl;
    return 1;
  }

  return 0;
}
