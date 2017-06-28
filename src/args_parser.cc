#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>

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

arg_type get_arg_type(const char* arg) {
  unsigned char n = 0;
  for (char c=arg[n]; c=='-'; c=arg[++n]) ;
  switch (n) {
    case  1:
      if (arg[2]!='\0') throw args_error(
        "short arg "+std::string(arg)+" more than one char long");
      return   short_arg;
    case  2: return    long_arg;
    default: return context_arg;
  }
}

}

void parser::parse(int argc, char const * const * argv) {
  using namespace ::ivanp::args::detail;
  // for (int i=1; i<argc; ++i) {
  //   for (const auto& m : help_matchers) {
  //     if ((*m)(argv[i])) {
  //       help();
  //       return;
  //     }
  //   }
  // }

  arg_def_base *waiting = nullptr;
  const char* str = nullptr;
  std::string tmp;

  for (int i=1; i<argc; ++i) {
    const char* arg = argv[i];

    const auto arg_type = get_arg_type(arg);
    cout << arg << ' ' << arg_type << endl;

    // ==============================================================
    if (arg_type!=context_arg) {
      if (waiting && waiting->need)
        throw args_error(waiting->name() + " without value");
    }

    switch (arg_type) {
      case long_arg: // ---------------------------------------------

        str = strchr(arg,'='); // split by '=' if long
        if (str) tmp.assign(arg,str), ++str, arg = tmp.c_str();
        // else waiting = nullptr;

        break;
      case short_arg: // --------------------------------------------

        break;
      case context_arg: // ------------------------------------------

        break;
    }

    // ==============================================================

    for (auto& m : matchers[arg_type]) {
      auto& def = m.second;
      cout << def->descr << endl;
      if ((*m.first)(arg)) {
        cout << arg << " matched with " << def->descr << endl;
        if (str) def->parse(str), str = nullptr; // call parser & reset
        else waiting = def;
        goto cont;
      }
    }

    if (waiting && waiting->count) {
      waiting->parse(arg);
      if (!waiting->count) waiting = nullptr;
      goto cont;
    }

    throw args_error(std::string("unexpected option ") + arg);
    cont: ;
  }
}

// FIXME
// void parser::help() {
//   cout << "help" << endl;
// }

}} // end namespace ivanp
