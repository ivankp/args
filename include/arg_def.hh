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

template <typename... Args> class switch_init {
  std::tuple<Args...> args;
  template <typename T, size_t... I>
  inline void construct(T& x, std::index_sequence<I...>) {
    x = { std::get<I>(args)... };
  }
public:
  template <typename... T>
  switch_init(std::tuple<T...>&& tup): args(std::move(tup)) { }
  template <typename T> inline void construct(T& x) {
    construct(x,std::index_sequence_for<Args...>{});
  }
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
template <typename... Args>
inline _::switch_init<std::decay_t<Args>...> switch_init(Args&&... args) {
  return { std::forward_as_tuple(std::forward<Args>(args)...) };
}

namespace detail {

// Argument definition objects --------------------------------------
// These provide common interface for invoking single argument parsers
// and assigning new values to recepients via pointers
// These are created as a result of calling parser::operator()

struct arg_def_base {
  std::string descr;
  unsigned count = 0;

  arg_def_base(std::string&& descr): descr(std::move(descr)) { }
  virtual ~arg_def_base() { }
  virtual void parse(const char* arg) = 0;
  virtual std::string name() const { return descr; } // FIXME
  virtual bool is_switch() = 0;
  virtual unsigned min() const noexcept = 0;
  virtual unsigned max() const noexcept = 0;
};

template <typename T, typename... Mixins>
class arg_def final: public arg_def_base, Mixins... {
  T *x; // recepient of parsed value

  using mixins = std::tuple<Mixins...>;
  template <template<typename> typename Pred>
  using index_t = first_index_of<Pred,mixins>;
  template <typename Seq>
  using mix_t = std::tuple_element_t<seq_head<Seq>::value,mixins>;

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
    ++count;
    return true;
  }
  template <typename U = T> inline std::enable_if_t<
    !std::is_same<U,bool>::value && switch_init_index::size(),
  bool> is_switch_impl() {
    mix_t<switch_init_index>::construct(*x);
    ++count;
    return true;
  }
  template <typename U = T> inline std::enable_if_t<
    !std::is_same<U,bool>::value && !switch_init_index::size(),
  bool> is_switch_impl() const noexcept { return false; }

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

  // min max --------------------------------------------------------
  inline unsigned min() const noexcept {
    return switch_init_index::size();
  }

  using multi_index = index_t<_::is_multi>;
  template <typename index = multi_index>
  inline std::enable_if_t<
    index::size()==1,
  unsigned> max() noexcept {
    return mix_t<multi_index>::num;
  }
  template <typename index = multi_index>
  inline std::enable_if_t<
    index::size()==0 && 
  unsigned> max() const noexcept { return 0; }

  inline unsigned max() const noexcept {

    return switch_init_index::size();
  }

    return std::integral_constant<unsigned,switch_init_index::size() >::value;
  }

  // ----------------------------------------------------------------
public:
  template <typename... M>
  arg_def(T* x, std::string&& descr, M&&... m)
  : arg_def_base(std::move(descr)), Mixins(std::forward<M>(m))..., x(x)
  { set_count(); }

  inline void parse(const char* arg) {
    parse_impl(arg);
    ++count;
  }

  inline bool is_switch() { return is_switch_impl(); }
  inline std::string name() const { return name_impl(); }
};

// Traits -----------------------------------------------------------
// CA. Can have arguments
// MA. Must have arguments
// CS. Can be a switch
// MS. Is switch only (must)
// MB. Must receive all arguments before next option

template <typename ArgDef>
struct argCA {};

// Factory ----------------------------------------------------------
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
