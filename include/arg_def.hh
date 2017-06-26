#ifndef IVANP_ARG_DEF_HH
#define IVANP_ARG_DEF_HH

#ifdef ARGS_PARSER_BOOST_LEXICAL_CAST
#include <boost/lexical_cast.hpp>
#else
#include <sstream>
#endif

namespace ivanp { namespace args {

// Arg def mixins ---------------------------------------------------

template <size_t I> struct tag { };
template <typename T> struct is_tag : std::false_type { };
template <size_t I> struct is_tag<tag<I>> : std::true_type { };

struct name { std::string name; };
template <typename T> struct is_name : std::false_type { };
template <> struct is_name<name> : std::true_type { };

struct multi { };
template <typename T> struct is_multi : std::false_type { };
template <> struct is_multi<multi> : std::true_type { };

struct pos { unsigned pos; };
template <typename T> struct is_pos : std::false_type { };
template <> struct is_pos<pos> : std::true_type { };

struct req { };
template <typename T> struct is_req : std::false_type { };
template <> struct is_req<req> : std::true_type { };

template <typename T> struct is_parser {
  template <typename F>
  using type = typename is_callable<F,const char*,T&>::type;
};

namespace detail {

// Argument definition objects --------------------------------------
// These provide common interface for invoking single argument parsers
// and assigning new values to recepients via pointers
// These are created as a result of calling parser::operator()

struct arg_def_base {
  std::string descr;
  arg_def_base(std::string&& descr): descr(std::move(descr)) { }
  virtual ~arg_def_base() { }
};

template <typename T, typename... Mixins>
struct arg_def final: arg_def_base, Mixins... {
  T *x; // recepient of parsed value

  template <typename... M>
  arg_def(T* x, std::string&& descr, M&&... m)
  : arg_def_base(std::move(descr)), Mixins(std::forward<M>(m))..., x(x) { }

  using mixins = std::tuple<Mixins...>;
  using parser_index = get_indices_of_t<
    ::ivanp::args::is_parser<T>::template type, mixins >;

  // use custom parser if passed
  template <typename index = parser_index>
  inline std::enable_if_t<index::size()==1>
  operator()(const char* arg) const {
    using parser_t = std::tuple_element_t<seq_head<parser_index>::value,mixins>;
    parser_t::operator()(arg,*x);
  }
  // otherwise use default parser
  template <typename index = parser_index>
  inline std::enable_if_t<index::size()==0>
  operator()(const char* arg) const {
#ifdef ARGS_PARSER_BOOST_LEXICAL_CAST
    *x = boost::lexical_cast<T>(arg);
#else
    std::stringstream(arg) >> *x;
#endif
  }
};

template <typename T, typename Tuple, size_t... I>
inline auto make_arg_def(
  T* x, std::string&& descr, Tuple&& tup, std::index_sequence<I...>
) {
  using type = arg_def<T,
    std::decay_t<std::tuple_element_t<I,std::decay_t<Tuple>>>... >;
  return new type( x, std::move(descr), std::get<I>(tup)... );
}

} // end namespace detail

}}

#endif
