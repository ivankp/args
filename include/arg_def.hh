#ifndef IVANP_ARG_DEF_HH
#define IVANP_ARG_DEF_HH

#include <sstream>

namespace ivanp { namespace args {

// Arg def mixins ---------------------------------------------------

template <size_t I> struct tag { };
template <typename T> struct is_tag : std::false_type { };
template <size_t I> struct is_tag<tag<I>> : std::true_type { };

struct named { std::string name; };
template <typename T> struct is_named : std::false_type { };
template <> struct is_named<named> : std::true_type { };

struct multi { };
template <typename T> struct is_multi : std::false_type { };
template <> struct is_multi<multi> : std::true_type { };

struct pos { unsigned pos; };
template <typename T> struct is_pos : std::false_type { };
template <> struct is_pos<pos> : std::true_type { };

struct req { };
template <typename T> struct is_req : std::false_type { };
template <> struct is_req<req> : std::true_type { };

// template <typename F, template T>
// using is_parser = typename is_callable<F,const char*,T*>::type;

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

  // template <template <typename> typename Pred>
  // using has = std::integral_constant< bool,
  //   get_indices_of_t< Pred, std::tuple<Mixins...> >::size()>;

  // template <template <typename> typename Pred>
  // static constexpr bool has() {
  //   return get_indices_of_t< Pred, std::tuple<Mixins...> >::size();
  // }

  using mixins = std::tuple<Mixins...>;
  using parser_index = get_indices_of_t<
    ::ivanp::args::is_parser<T>::template type, mixins >;

  template <typename index = parser_index>
  inline std::enable_if_t<index::size()==1>
  operator()(const char* arg) const {
    using parser_t = std::tuple_element_t<seq_head<parser_index>::value,mixins>;
    parser_t::operator()(arg,*x);
  }
  template <typename index = parser_index>
  inline std::enable_if_t<index::size()==0>
  operator()(const char* arg) const { std::stringstream(arg) >> *x; }
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
