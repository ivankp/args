#ifndef IVANP_ARG_DEF_HH
#define IVANP_ARG_DEF_HH

#include "default_arg_parser.hh"

namespace ivanp { namespace args {

// Arg def mixins ---------------------------------------------------

namespace _ {

struct name { std::string name; };
template <typename T> struct is_name : std::false_type { };
template <> struct is_name<name> : std::true_type { };

struct multi { unsigned num = -1u; };
template <typename T> struct is_multi : std::false_type { };
template <> struct is_multi<multi> : std::true_type { };

struct pos { };
template <typename T> struct is_pos : std::false_type { };
template <> struct is_pos<pos> : std::true_type { };

struct req { };
template <typename T> struct is_req : std::false_type { };
template <> struct is_req<req> : std::true_type { };

template <typename... Args> struct switch_init {
  // std::tuple<decay_t<Args>...> args;
  // switch_init(
};
template <> struct switch_init<> {
  template <typename T> inline void operator()(T& x) const { x = { }; }
};
template <typename T> struct is_switch_init : std::false_type { };
template <typename... T>
struct is_switch_init<switch_init<T...>> : std::true_type { };

template <typename T> struct is_parser {
  template <typename F>
  using type = typename is_callable<F,const char*,T&>::type;
};

} // end namespace _

template <typename... Args>
inline _::name name(Args&&... args) { return {{std::forward<Args>(args)...}}; }
constexpr _::multi multi(unsigned n) noexcept { return {n}; }
constexpr _::multi multi() noexcept { return {}; }
constexpr _::pos pos() noexcept { return {}; }
constexpr _::req req() noexcept { return {}; }

namespace detail {

// Argument definition objects --------------------------------------
// These provide common interface for invoking single argument parsers
// and assigning new values to recepients via pointers
// These are created as a result of calling parser::operator()

struct arg_def_base {
  std::string descr;
  unsigned count = 1;
  bool need = false;

  arg_def_base(std::string&& descr): descr(std::move(descr)) { }
  virtual ~arg_def_base() { }
  virtual void parse(const char* arg) = 0;
  virtual std::string name() const { return descr; } // FIXME
  virtual bool is_switch() = 0;
};

template <typename T, typename... Mixins>
class arg_def final: public arg_def_base, Mixins... {
  T *x; // recepient of parsed value

  using mixins = std::tuple<Mixins...>;
  template <typename Seq>
  using mix_t = std::tuple_element_t<seq_head<Seq>::value,mixins>;
  template <template<typename> typename Is>
  using index_t = get_indices_of_t<Is,mixins>;

  // parser ---------------------------------------------------------
  using parser_index = index_t<_::is_parser<T>::template type>;
  template <typename index = parser_index>
  inline std::enable_if_t<index::size()==1>
  parse_impl(const char* arg) const {
    mix_t<index>::operator()(arg,*x);
  }
  template <typename index = parser_index>
  inline std::enable_if_t<index::size()==0>
  parse_impl(const char* arg) const {
    arg_parser<T>::parse(arg,*x);
  }

  // switch ---------------------------------------------------------
  using switch_init_index = index_t<_::is_switch_init>;
  template <typename U = T> inline std::enable_if_t<
    std::is_same<U,bool>::value,
  bool> is_switch_impl() noexcept {
    (*x) = true;
    --count;
    return true;
  }
  template <typename U = T> inline std::enable_if_t<
    !std::is_same<U,bool>::value && switch_init_index::size(),
  bool> is_switch_impl() {
    mix_t<switch_init_index>::operator()(*x);
    --count;
    return true;
  }
  template <typename U = T> inline std::enable_if_t<
    !std::is_same<U,bool>::value && !switch_init_index::size(),
  bool> is_switch_impl() const noexcept { return false; }

  // multi ----------------------------------------------------------
  using multi_index = index_t<_::is_multi>;
  template <typename index = multi_index>
  inline std::enable_if_t<index::size()==1> set_count() noexcept {
    count = mix_t<multi_index>::num;
  }
  template <typename index = multi_index>
  inline std::enable_if_t<index::size()==0> set_count() const noexcept { }

  // name -----------------------------------------------------------
  using name_index = index_t<_::is_name>;
  template <typename index = name_index>
  inline std::enable_if_t<index::size()==1,std::string> name_impl() const {
    return mix_t<index>::name;
  }
  template <typename index = name_index>
  inline std::enable_if_t<index::size()==0,std::string> name_impl() const {
    return arg_def_base::name();
  }

  // ----------------------------------------------------------------
public:
  template <typename... M>
  arg_def(T* x, std::string&& descr, M&&... m)
  : arg_def_base(std::move(descr)), Mixins(std::forward<M>(m))..., x(x)
  { set_count(); }

  inline void parse(const char* arg) {
    parse_impl(arg);
    --count;
  }

  inline bool is_switch() { return is_switch_impl(); }
  inline std::string name() const { return name_impl(); }
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
