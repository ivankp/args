#ifndef IVANP_ARGS_PARSER_HH
#define IVANP_ARGS_PARSER_HH

#include <string>
#include <vector>
#include <memory>

#ifdef ARGS_PARSER_STD_REGEX
#include <regex>
#elif defined(ARGS_PARSER_BOOST_REGEX)
#include <boost/regex.hpp>
#endif

namespace ivanp { namespace args {

namespace detail {

// Matchers ---------------------------------------------------------

struct arg_match_base {
  virtual bool operator()(const char* arg) const noexcept = 0;
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

// ------------------------------------------------------------------

class arg_def_base {
  const char *_descr;
protected:
  constexpr arg_def_base(const char *descr): _descr(descr) { }
public:
  virtual void operator()(char const * const * arg) const = 0;
  inline const char* descr() const noexcept { return _descr; }
};

template <typename T>
class arg_def final: public arg_def_base {
  T *x;
  // std::function<void(char const * const *,T*)> parser;
public:
  // constexpr arg_def(T* x, const char *descr): arg_def_base{descr}, x(x) { }
  constexpr arg_def(T* x, const char *descr): arg_def_base(descr), x(x) { }
  inline void operator()(char const * const * arg) const {
    // parser(strp,x);
  }
};

// ------------------------------------------------------------------

} // end namespace detail

class parser {
  std::vector<std::unique_ptr<detail::arg_def_base>> arg_defs;
  std::vector<std::vector<std::pair<
    std::unique_ptr<const detail::arg_match_base>,
    const detail::arg_def_base*
  >>> matchers;

public:
  parser(): matchers(3) { }
  void parse(int argc, char const * const * argv);

  template <typename T, typename... Props>
  parser& operator()(T* x,
    std::initializer_list<const char*> matchers, const char* descr="",
    const Props&... props
  ) {
    arg_defs.emplace_back(new detail::arg_def<T>(x,descr));
    for (const char* x : matchers) {
      auto&& m = detail::make_arg_match(x);
      this->matchers[m.second].emplace_back(m.first,arg_defs.back().get());
    }
    return *this;
  }

  template <typename T, typename Matcher, typename... Props>
  parser& operator()(T* x,
    Matcher&& matcher, const char* descr="",
    const Props&... props
  ) {
    arg_defs.emplace_back(new detail::arg_def<T>(x,descr));
    auto&& m = detail::make_arg_match(std::forward<Matcher>(matcher));
    matchers[m.second].emplace_back(m.first,arg_defs.back().get());
    return *this;
  }

/*
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
