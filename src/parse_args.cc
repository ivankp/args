#include "parse_args.hh"

#include <iostream>

namespace ivanp { namespace args {

namespace detail {

template <>
bool arg_match<char>::operator()(const char* arg) const noexcept {
  return ( arg[0]=='-' && arg[1]==m && arg[2]=='\0' );
}
template <>
bool arg_match<const char*>::operator()(const char* arg) const noexcept {
  for (int i=0; m[i]!='\0' && arg[i]!='\0'; ++i)
    if ( arg[i]!=m[i] ) return false;
  return true;
}
template <>
bool arg_match<std::string>::operator()(const char* arg) const noexcept {
  for (int i=0; m[i]!='\0' && arg[i]!='\0'; ++i)
    if ( arg[i]!=m[i] ) return false;
  return true;
}
template <>
bool arg_match<std::regex>::operator()(const char* arg) const noexcept {
  return std::regex_match(arg,m);
}

arg_type find_arg_type(const char* arg) noexcept {
  unsigned char n = 0;
  for (char c=*arg; c=='-'; ++arg) ++n;
  switch (n) {
    case  1: return   short_arg;
    case  2: return    long_arg;
    default: return context_arg;
  }
}

}

void parse_args::parse(int argc, char const * const * argv) {
  for (int i=1; i<argc; ++i) {
    std::cout << argv[i] << std::endl;
    for (const auto& m : matchers[detail::find_arg_type(argv[i])]) {
      if ((*m.first)(argv[i])) {
        std::cout << argv[i] << " matched with " << m.second->descr() << std::endl;
        break;
      }
    }
    std::cout << argv[i] << " didn't match" << std::endl;
  }
}

}} // end namespace ivanp
