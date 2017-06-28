#include <iostream>
#include <cstring>

// #define ARGS_PARSER_STD_REGEX
// #define ARGS_PARSER_BOOST_LEXICAL_CAST
#include "args_parser.hh"

using std::cout;
using std::cerr;
using std::endl;
using namespace std::string_literals;

int main(int argc, char* argv[]) {
  double d;
  int i;
  std::string s;

  try {
    using namespace ivanp::args;
    parser()
      (&d,'d',"Double")
      (&i,{"-i","--int"},"Int",pos{},multi{})
      (&i,"--count","Count",
        [](const char* str, int& x){ x = strlen(str); })
      (&s,std::forward_as_tuple(
            's', [](const char* arg){ return arg[0]=='t'; }),
          "starts with \'t\'")
      // (&c,".*\\.txt","ends with .txt",name{"regex"})
      .parse(argc,argv);
  } catch(const std::exception& e) {
    cerr <<"\033[31m"<< e.what() <<"\033[0m"<< endl;
    return 1;
  }

  TEST( d )
  TEST( i )
  TEST( s )

  return 0;
}
