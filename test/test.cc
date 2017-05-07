#include <iostream>
// #include <regex>

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
    (&a,std::string("-aa"),"A")
    (&b,{"-b","--b-opt"},"B")
    (&c,'c',"C")
    (&c,[](const char* arg){ return arg[0]=='t'; },"starts with \'t\'")
    (&c,".*\\.txt","R")
    .parse(argc,argv);
  } catch(std::exception& e) {
    cerr << e.what() << endl;
    return 1;
  }

  return 0;
}
