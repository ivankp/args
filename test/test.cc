#include <iostream>

#define TEST(var) \
  std::cout <<"\033[36m"<< #var <<"\033[0m"<< " = " << var << std::endl;

// #define ARGS_PARSER_STD_REGEX
#include "args_parser.hh"

using std::cout;
using std::cerr;
using std::endl;
using namespace std::string_literals;

int main(int argc, char* argv[]) {
  int a, b, c;

  try {
    using namespace ivanp::args::prop;
    ivanp::args::parser('h',"--help")
    (&a,"-aa"s,"A")
    (&b,{"-b","--b-opt"},"B",mult{},pos{2},name{"b"},4,4,4)
    (&c,'c',"C")
    (&c,[](const char* arg){ return arg[0]=='t'; },"starts with \'t\'")
    (&c,".*\\.txt","R",name{"regex"},req())
    .parse(argc,argv);
  } catch(std::exception& e) {
    cerr << e.what() << endl;
    return 1;
  }

  return 0;
}
