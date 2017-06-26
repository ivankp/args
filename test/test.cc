#include <iostream>

#define TEST(var) \
  std::cout <<"\033[36m"<< #var <<"\033[0m"<< " = " << var << std::endl;

// #define ARGS_PARSER_STD_REGEX
// #define ARGS_PARSER_BOOST_LEXICAL_CAST
#include "args_parser.hh"

using std::cout;
using std::cerr;
using std::endl;
using namespace std::string_literals;

int main(int argc, char* argv[]) {
  int a, b, c;

  try {
    using namespace ivanp::args;
    parser()
      (&a,"-aa"s,"A")
      (&b,{"-b","--b-opt"},"B",multi{},pos{2})
      (&c,'c',"C",[](const char* str,int&){ cout << str << endl; })
      (&c,std::forward_as_tuple(
            't', [](const char* arg){ return arg[0]=='t'; }
          ),"starts with \'t\'")
      (&c,".*\\.txt","ends with .txt",name{"regex"})
      .parse(argc,argv);
  } catch(const std::exception& e) {
    cerr << e.what() << endl;
    return 1;
  }

  return 0;
}
