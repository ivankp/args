#include <iostream>
#include <string>
#include <cstring>

#include "args_parser.hh"

using std::cout;
using std::cerr;
using std::endl;

namespace ivanp { namespace args {

namespace detail {

template <>
bool arg_match<const char*>::operator()(const char* arg) const noexcept {
  int i = 0;
  for (; m[i]!='\0' && arg[i]!='\0'; ++i)
    if ( arg[i]!=m[i] ) return false;
  return m[i]=='\0' && arg[i]=='\0';
}

arg_type get_arg_type(const char* arg) noexcept {
  unsigned char n = 0;
  for (char c=arg[n]; c=='-'; c=arg[++n]) ;
  switch (n) {
    case  1: return   short_arg;
    case  2: return    long_arg;
    default: return context_arg;
  }
}

}

void parser::parse(int argc, char const * const * argv) {
  // for (int i=1; i<argc; ++i) {
  //   for (const auto& m : help_matchers) {
  //     if ((*m)(argv[i])) {
  //       help();
  //       return;
  //     }
  //   }
  // }

  const char* str = nullptr;
  std::string tmp;

  for (int i=1; i<argc; ++i) {
    const char* arg = argv[i];
    using namespace ::ivanp::args::detail;

    const auto arg_type = get_arg_type(arg);
    cout << arg << ' ' << arg_type << endl;

    if (arg_type==long_arg) { // split by '=' if long
      str = strchr(arg,'=');
      if (str) tmp.assign(arg,str), ++str, arg = tmp.c_str();
    }

    for (const auto& m : matchers[arg_type]) {
      cout << m.second->descr << endl;
      if ((*m.first)(arg)) {
        cout << arg << " matched with " << m.second->descr << endl;
        if (str) m.second->parse(str), str = nullptr; // call parser & reset
        goto outer;
      }
    }
    cout << arg << " didn't match" << endl;
    outer: ;
  }
}

// FIXME
// void parser::help() {
//   cout << "help" << endl;
// }

}} // end namespace ivanp
