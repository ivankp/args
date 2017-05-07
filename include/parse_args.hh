#ifndef IVANP_PARSE_ARGS_HH
#define IVANP_PARSE_ARGS_HH

#include <string>
#include <vector>
#include <memory>

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
bool arg_match<char>::operator()(const char* arg) const noexcept;
template <>
bool arg_match<const char*>::operator()(const char* arg) const noexcept;
template <>
bool arg_match<std::string>::operator()(const char* arg) const noexcept;
#ifdef _GLIBCXX_REGEX
template <>
inline bool arg_match<std::regex>::operator()(const char* arg) const noexcept {
  return std::regex_match(arg,m);
}
#endif

// ------------------------------------------------------------------

enum arg_type { long_arg, short_arg, context_arg };

arg_type find_arg_type(const char* arg) noexcept;

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
template <typename T>
arg_match_type make_arg_match_impl(T&& x, arg_match_tag<const char*>) noexcept {
  const arg_type t = find_arg_type(x);
  const arg_match_base *m = nullptr;
  switch (t) {
    case    long_arg: m = new arg_match<const char*>( x );
    case   short_arg: m = new arg_match<char>( x[1] );
    case context_arg: 
#ifdef _GLIBCXX_REGEX
      m = new arg_match<std::regex>( x );
#else
      m = new arg_match<const char*>( x );
#endif
  }
  return { m, t };
}
template <typename T>
arg_match_type make_arg_match_impl(T&& x, arg_match_tag<std::string>) noexcept {
  const arg_type t = find_arg_type(x.c_str());
  const arg_match_base *m = nullptr;
  switch (t) {
    case    long_arg: m = new arg_match<std::string>( std::forward<T>(x) );
    case   short_arg: m = new arg_match<char>( x[1] );
    case context_arg: 
#ifdef _GLIBCXX_REGEX
      m = new arg_match<std::regex>( std::forward<T>(x) );
#else
      m = new arg_match<std::string>( std::forward<T>(x) );
#endif
  }
  return { m, t };
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

class parse_args {
  std::vector<std::unique_ptr<detail::arg_def_base>> arg_defs;
  std::vector<std::vector<std::pair<
    std::unique_ptr<const detail::arg_match_base>,
    const detail::arg_def_base*
  >>> matchers;

public:
  parse_args(): matchers(3) { }
  void parse(int argc, char const * const * argv);

  template <typename T, typename... Props>
  parse_args& operator()(T* x,
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
  parse_args& operator()(T* x,
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
  parse_args& operator()(T* x,
    std::tuple<Matchers...> matchers, const char* descr="",
    const Props&... props
  ) {
    return *this;
  }
*/
};

}} // end namespace ivanp

#endif
