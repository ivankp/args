#include "args_parser.hh"

#include <iostream>

namespace ivanp { namespace args {

namespace detail {

template <>
bool arg_match<const char*>::operator()(const char* arg) const noexcept {
  int i = 0;
  for (; m[i]!='\0' && arg[i]!='\0'; ++i)
    if ( arg[i]!=m[i] ) return false;
  return m[i]=='\0' && arg[i]=='\0';
}

arg_type find_arg_type(const char* arg) noexcept {
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
  for (int i=1; i<argc; ++i) {
    std::cout << argv[i] << ' ' << detail::find_arg_type(argv[i]) << std::endl;
    for (const auto& m : matchers[detail::find_arg_type(argv[i])]) {
      std::cout << m.second->descr << std::endl;
      if ((*m.first)(argv[i])) {
        std::cout << argv[i] << " matched with " << m.second->descr << std::endl;
        goto outer;
      }
    }
    std::cout << argv[i] << " didn't match" << std::endl;
    outer: ;
  }
}

}} // end namespace ivanp
