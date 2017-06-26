#ifndef IVANP_UTILITY_HH
#define IVANP_UTILITY_HH

namespace ivanp {

template <typename... T> struct make_void { typedef void type; };
template <typename... T> using void_t = typename make_void<T...>::type;

template <template<typename> typename Pred, typename Tuple>
class get_indices_of {
  static constexpr size_t size = std::tuple_size<Tuple>::value;
  template <size_t I, size_t... II> struct impl {
    using type = std::conditional_t<
      Pred<std::tuple_element_t<I,Tuple>>::value,
      typename impl<I+1, II..., I>::type,
      typename impl<I+1, II...>::type >;
  };
  template <size_t... II> struct impl<size,II...> {
    using type = std::index_sequence<II...>;
  };
public:
  using type = typename impl<0>::type;
};
template <template<typename> typename Pred, typename Tuple>
using get_indices_of_t = typename get_indices_of<Pred,Tuple>::type;

template <typename... SS> struct seq_join;
template <typename T, size_t... I1, size_t... I2>
struct seq_join<std::integer_sequence<T,I1...>,std::integer_sequence<T,I2...>> {
  using type = std::integer_sequence<T,I1...,I2...>;
};
template <typename S1, typename S2, typename... SS>
struct seq_join<S1,S2,SS...>
: seq_join<typename seq_join<S1,S2>::type,SS...> { };
template <typename... SS>
using seq_join_t = typename seq_join<SS...>::type;

template <typename T> struct seq_head;
template <typename T, T Head, T... I>
struct seq_head<std::integer_sequence<T,Head,I...>>
: std::integral_constant<T,Head> { };

template <template <typename...> typename Pred, typename Tuple, typename Seq>
struct meta_apply;
template <template <typename...> typename Pred, typename... T, size_t... I>
struct meta_apply<Pred,std::tuple<T...>,std::index_sequence<I...>> {
  using type = Pred<std::tuple_element_t<I,std::tuple<T...>>...>;
};
template <template <typename...> typename Pred, typename Tuple, typename Seq>
using meta_apply_t = typename meta_apply<Pred,Tuple,Seq>::type;

template <typename T, typename... Args> // x(args...)
class is_callable {
  template <typename, typename = void>
  struct impl: std::false_type { };
  template <typename U>
  struct impl<U,
    void_t<decltype( std::declval<U&>()(std::declval<Args>()...) )>
  > : std::true_type { };
public:
  using type = impl<T>;
  static constexpr bool value = type::value;
};

} // end namespace ivanp

#endif
