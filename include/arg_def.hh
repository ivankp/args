#ifndef IVANP_ARG_DEF_HH
#define IVANP_ARG_DEF_HH

namespace ivanp { namespace args {
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

  // FIXME
  // template <typename U> using has = std::integral_constant<bool,
  //   get_indices_of_t<
  //     meta_bind1_t<std::is_same,U>, std::tuple<Mixins...>
  //   >::size()>;

  inline void operator()(char const * const * arg) const {
    // parser(strp,x);
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

}}

#endif
