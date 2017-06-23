#ifndef IVANP_ARGS_PARSER_HH
#define IVANP_ARGS_PARSER_HH

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <type_traits>

#ifdef ARGS_PARSER_STD_REGEX
#include <regex>
#elif defined(ARGS_PARSER_BOOST_REGEX)
#include <boost/regex.hpp>
#endif

#include "string.hh"

namespace ivanp { namespace args {

namespace detail {

// Matchers ---------------------------------------------------------
// These represent rules for matching raw args with recepient pointers

struct arg_match_base {
  virtual bool operator()(const char* arg) const noexcept = 0;
  virtual ~arg_match_base() { }
};

template <typename T>
class arg_match final : public arg_match_base {
  T m;
public:
  template <typename... Args>
  arg_match(Args&&... args): m(std::forward<Args>(args)...) { }
  inline bool operator()(const char* arg) const noexcept { return m(arg); }
};

template <>
inline bool arg_match<char>::operator()(const char* arg) const noexcept {
  return ( arg[1]==m && arg[2]=='\0' );
}
template <>
bool arg_match<const char*>::operator()(const char* arg) const noexcept;
template <>
inline bool arg_match<std::string>::operator()(const char* arg) const noexcept {
  return m == arg;
}
#ifdef _GLIBCXX_REGEX
template <>
inline bool arg_match<std::regex>::operator()(const char* arg) const noexcept {
  return std::regex_match(arg,m);
}
#endif
#ifdef BOOST_RE_REGEX_HPP
template <>
inline bool arg_match<boost::regex>::operator()(const char* arg) const noexcept {
  return boost::regex_match(arg,m);
}
#endif

// ------------------------------------------------------------------

enum arg_type { long_arg, short_arg, context_arg };

arg_type find_arg_type(const char* arg) noexcept;
inline arg_type find_arg_type(const std::string& arg) noexcept {
  return find_arg_type(arg.c_str());
}

// Matcher factories ------------------------------------------------

using arg_match_type = std::pair<const arg_match_base*,arg_type>;
template <typename T> struct arg_match_tag { };

template <typename T>
inline arg_match_type make_arg_match(T&& x) noexcept {
  return make_arg_match_impl(std::forward<T>(x),
    arg_match_tag<std::decay_t<T>>{});
}

template <typename T, typename Tag>
arg_match_type make_arg_match_impl(T&& x, Tag) noexcept {
  return { new arg_match<T>( std::forward<T>(x) ), context_arg };
}
template <typename T>
arg_match_type make_arg_match_impl(T&& x, arg_match_tag<char>) noexcept {
  return { new arg_match<char>( x ), short_arg };
}
template <typename T, typename TagT>
std::enable_if_t<std::is_convertible<TagT,std::string>::value,arg_match_type>
make_arg_match_impl(T&& x, arg_match_tag<TagT>) noexcept {
  const arg_type t = find_arg_type(x);
#if defined(ARGS_PARSER_STD_REGEX) || defined(ARGS_PARSER_BOOST_REGEX)
  using regex_t =
# ifdef ARGS_PARSER_STD_REGEX
    std::regex;
# else
    boost::regex;
# endif
  if (t==context_arg)
    return { new arg_match<regex_t>( std::forward<T>(x) ), t };
  else
#endif
    return { new arg_match<TagT>( std::forward<T>(x) ), t };
}

// Argument definition objects --------------------------------------
// These provide common interface for invoking single argument parsers
//   and assigning new values to recepients via pointers
// These are created as a result of calling parser::operator(. . .)

class arg_def_base {
public:
  bool req  : 1; // required argument
  bool mult : 1; // allow repeated values
  bool pos  : 1; // is a positional argument
  bool adj  : 1; // consider only the first group of adjacent arguments
                 // in positional context
  unsigned char npos : 4; // consider at most this many arguments
                          // in positional context
  std::string descr, def, name;

  virtual void operator()(char const * const * arg) const = 0;
  virtual std::string str() const = 0;
protected:
  template <typename S>
  arg_def_base(S&& descr): req(0), mult(0), pos(0), adj(0), npos(0),
                           descr(std::forward<S>(descr)) { }
// private:
//   template <typename T>
//   inline void set_mult() noexcept { mult = false; }
// TODO: default falues for properties
};

template <typename T>
class arg_def final: public arg_def_base {
  T *x; // recepient of parsed value
  // std::function<void(char const * const *,T*)> parser;
public:
  template <typename S>
  arg_def(T* x, S&& descr): arg_def_base(std::forward<S>(descr)), x(x) { }
  inline void operator()(char const * const * arg) const {
    // parser(strp,x);
  }
  inline std::string str() const { return to_str_if_can(*x); }
};

// ------------------------------------------------------------------

} // end namespace detail

// ------------------------------------------------------------------
// Visitors modifying arg_def flags
namespace prop {

struct prop {
protected:
  using arg_def = ::ivanp::args::detail::arg_def_base;
};

struct req : prop { void operator()(arg_def* arg) { arg->req  = true; } };
struct mult: prop { void operator()(arg_def* arg) { arg->mult = true; } };
struct adj : prop { void operator()(arg_def* arg) { arg->adj  = true; } };
struct pos : prop {
  unsigned char npos = 0;
  pos(unsigned char n): npos(n) { }
  void operator()(arg_def* arg) { arg->pos = true; arg->npos = npos; }
};
struct name: prop {
  std::string str;
  template <typename... Args>
  name(Args&&... args): str(std::forward<Args>(args)...) { }
  void operator()(arg_def* arg) { arg->name = std::move(str); }
};
struct def : prop {
  std::string str;
  template <typename... Args>
  def(Args&&... args): str(std::forward<Args>(args)...) { }
  void operator()(arg_def* arg) {
    arg->def = '(' + str.size() ? str : arg->str() + ')';
  }
};

namespace detail {

template <typename Props, size_t At>
using is_prop = std::is_base_of< prop, std::tuple_element_t<At,Props> >;

template <typename Props, size_t At>
constexpr bool same_type() { return false; }
template <typename Props, size_t At, size_t I1, size_t... I>
constexpr bool same_type() {
  return std::is_same<
      std::tuple_element_t<At,Props>,std::tuple_element_t<I1,Props>
    >::value || same_type<Props,At,I...>();
}

template <typename Props, size_t At, size_t... I>
struct get_pred_indices_impl {
  static_assert(!same_type<Props,At,I...>(),
    "\033[33m""repeated prop type in argument definition""\033[0m");
  using type = typename std::conditional_t< is_prop<Props,At>::value,
    get_pred_indices_impl<Props,At-1,At,I...>,
    get_pred_indices_impl<Props,At-1,I...> >::type;
};
template <typename Props, size_t... I>
struct get_pred_indices_impl<Props,0,I...> {
  static_assert(!same_type<Props,0,I...>(),
    "\033[33m""repeated prop type in argument definition""\033[0m");
  using type = std::conditional_t< is_prop<Props,0>::value,
      std::index_sequence<0,I...>,
      std::index_sequence<I...> >;
};
template <size_t At>
struct get_pred_indices_impl<std::tuple<>,At> {
  using type = std::index_sequence<>;
};
template <typename... Props>
using get_pred_indices = typename get_pred_indices_impl<
  std::tuple<Props...>, sizeof...(Props)-1 >::type;

using arg_def = ::ivanp::args::detail::arg_def_base;

template <size_t I1, typename Props>
inline void apply_all_impl(arg_def* arg, Props&& props) {
  std::get<I1>(props)(arg);
}
template <size_t I1, size_t... I, typename Props>
inline std::enable_if_t<sizeof...(I)> apply_all_impl(arg_def* arg, Props&& props) {
  std::get<I1>(props)(arg);
  apply_all_impl<I...>(arg,props);
}
template <size_t... I, typename Props>
inline void apply_all(arg_def* arg, Props&& props, std::index_sequence<I...>) {
  apply_all_impl<I...>(arg,props);
}
template <typename Props>
inline void apply_all(arg_def* arg, Props&& props, std::index_sequence<>) { }

} // end namespace prop::detail
} // end namespace prop
// ------------------------------------------------------------------

class parser {
  std::vector<std::unique_ptr<detail::arg_def_base>> arg_defs;
  std::array<std::vector<std::pair<
    std::unique_ptr<const detail::arg_match_base>,
    const detail::arg_def_base*
  >>,3> matchers;
  std::vector<const detail::arg_match_base*> help_matchers;

  // detail::arg_def_base *_last_arg = nullptr;
  // TODO: cross communication between arg defs

public:
  template <typename... M>
  parser(M&&... m): help_matchers{
      new detail::arg_match<std::decay_t<M>>(std::forward<M>(m))...
    } { }
  ~parser() { // because you can't list initialize vector of un-copiable objects
    for (auto* ptr : help_matchers) delete ptr;
  }

  void parse(int argc, char const * const * argv);
  void help();

  template <typename T, typename... Props>
  parser& operator()(T* x,
    std::initializer_list<const char*> matchers, const char* descr="",
    Props&&... props
  ) {
    auto *arg = new detail::arg_def<T>(x,descr);
    arg_defs.emplace_back(arg);
    for (const char* x : matchers) {
      arg->name += to_str_if_can(x);
      if (&x != &*matchers.end()) arg->name += ", ";
      auto&& m = detail::make_arg_match(x);
      // TODO: find out what binding to auto&& really does
      this->matchers[m.second].emplace_back(m.first,arg);
    }
    auto props_tup = std::make_tuple(std::forward<Props>(props)...);
    using pred_indices = prop::detail::get_pred_indices<Props...>;
    prop::detail::apply_all( arg, props_tup, pred_indices{} );
    return *this;
  }

  template <typename T, typename Matcher, typename... Props>
  parser& operator()(T* x,
    Matcher&& matcher, const char* descr="", Props&&... props
  ) {
    auto *arg = new detail::arg_def<T>(x,descr);
    arg->name = to_str_if_can(matcher);
    arg_defs.emplace_back(arg);
    auto&& m = detail::make_arg_match(std::forward<Matcher>(matcher));
    matchers[m.second].emplace_back(m.first,arg);
    auto props_tup = std::make_tuple(std::forward<Props>(props)...);
    using pred_indices = prop::detail::get_pred_indices<Props...>;
    prop::detail::apply_all( arg, props_tup, pred_indices{} );
    return *this;
  }

/* FIXME
  template <typename T, typename... Matchers, typename... Props>
  parser& operator()(T* x,
    std::tuple<Matchers...> matchers, const char* descr="",
    const Props&... props
  ) {
    return *this;
  }
*/
};

}} // end namespace ivanp

#endif
