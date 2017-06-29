#ifndef IVANP_DEFAULT_ARG_PARSER_HH
#define IVANP_DEFAULT_ARG_PARSER_HH

#ifdef ARGS_PARSER_BOOST_LEXICAL_CAST
// #if __has_include(<boost/lexical_cast.hpp>)
#include <boost/lexical_cast.hpp>
#else
#include <sstream>
#endif

namespace ivanp { namespace args {
namespace detail {

template <typename T> struct arg_parser {
  inline static void parse(const char* arg, T& x) {
#ifdef ARGS_PARSER_BOOST_LEXICAL_CAST
    try {
      x = boost::lexical_cast<T>(arg);
    } catch (...) {
      throw args::error(cat(
        '\"',arg,"\" cannot be interpreted as ",type_str<T>()));
    }
#else
    std::istringstream(arg) >> x;
#endif
  }
};

}
}}

#endif
