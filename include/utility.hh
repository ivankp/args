#ifndef IVANP_UTILITY_HH
#define IVANP_UTILITY_HH

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

template <template <typename...> typename Pred, typename Tuple, typename Seq>
struct meta_apply;
template <template <typename...> typename Pred, typename... T, size_t... I>
struct meta_apply<Pred,std::tuple<T...>,std::index_sequence<I...>> {
  using type = Pred<std::tuple_element_t<I,std::tuple<T...>>...>;
};
template <template <typename...> typename Pred, typename Tuple, typename Seq>
using meta_apply_t = typename meta_apply<Pred,Tuple,Seq>::type;

template <template <typename...> typename F, typename T>
struct meta_bind1 {
  template <typename... Args> using type = F<T,Args...>;
};
template <template <typename...> typename F, typename T>
using meta_bind1_t = typename meta_bind1<F,T>::type;

#endif
