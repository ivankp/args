#ifndef IVANP_ARGS_PARSER_HH
#define IVANP_ARGS_PARSER_HH

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <type_traits>
#include <stdexcept>

#include "type.hh"

#include "utility.hh"
#include "arg_match.hh"
#include "arg_def.hh"

namespace ivanp { namespace args {

class parser {
  std::vector<std::unique_ptr<detail::arg_def_base>> arg_defs;
  std::array<std::vector<std::pair<
    std::unique_ptr<const detail::arg_match_base>,
    const detail::arg_def_base*
  >>,3> matchers;

  template <typename T, typename... Props>
  inline auto* add_arg_def(T* x, std::string&& descr, Props&&... p) {
    using props_types = std::tuple<std::decay_t<Props>...>;
    const auto props  = std::forward_as_tuple(std::forward<Props>(p)...);

#define UNIQUE_PROP_ASSERT(NAME) \
    using NAME##_i = get_indices_of_t< \
      ::ivanp::args::is_##NAME, props_types>; \
    static_assert( NAME##_i::size() <= 1, \
      "\033[33mrepeated \"" #NAME "\" in program argument definition\033[0m");

    UNIQUE_PROP_ASSERT(name)
    UNIQUE_PROP_ASSERT(pos)
    UNIQUE_PROP_ASSERT(req)
    UNIQUE_PROP_ASSERT(multi)
    UNIQUE_PROP_ASSERT(tag)

#undef UNIQUE_PROP_ASSERT

    using parser_i = get_indices_of_t<
      ::ivanp::args::is_parser<T>::template type, props_types>;
    static_assert( parser_i::size() <= 1,
      "\033[33mrepeated parser in program argument definition\033[0m");

    using seq = seq_join_t< parser_i, name_i, pos_i, req_i, multi_i, tag_i >;

    static_assert( seq::size() == sizeof...(Props),
      "\033[33munrecognized option in program argument definition\033[0m");

    auto *arg_def = detail::make_arg_def(x, std::move(descr), props, seq{});
    arg_defs.emplace_back(arg_def);

    using arg_def_t = std::decay_t<decltype(*arg_def)>;
    type_size<arg_def_t>();

    return arg_def;
  }

  template <typename Matcher>
  inline void add_arg_match(
    Matcher&& matcher, detail::arg_def_base* arg_def
  ) {
    auto&& m = detail::make_arg_match(std::forward<Matcher>(matcher));
    matchers[m.second].emplace_back(std::move(m.first),arg_def);
  }
  template <typename... M, size_t... I>
  inline void add_arg_matches(
    const std::tuple<M...>& matchers, detail::arg_def_base* arg_def,
    std::index_sequence<I...>
  ) {
#ifdef __cpp_fold_expressions
    (add_arg_match(std::get<I>(matchers),arg_def),...);
#else
    fold((add_arg_match(std::get<I>(matchers),arg_def),0)...);
#endif
  }

public:
  void parse(int argc, char const * const * argv);
  // void help(); // FIXME

  template <typename T, typename... Props>
  parser& operator()(T* x,
    std::initializer_list<const char*> matchers,
    std::string descr={}, Props&&... p
  ) {
    if (matchers.size()==0) throw std::invalid_argument(
      "empty initializer list in program argument definition");
    auto *arg_def = add_arg_def(x,std::move(descr),std::forward<Props>(p)...);
    for (const char* m : matchers) add_arg_match(m,arg_def);
    return *this;
  }

  template <typename T, typename Matcher, typename... Props>
  std::enable_if_t<!is_tuple<std::decay_t<Matcher>>::value,parser&>
  operator()(T* x,
    Matcher&& matcher,
    std::string descr={}, Props&&... p
  ) {
    auto *arg_def = add_arg_def(x,std::move(descr),std::forward<Props>(p)...);
    add_arg_match(matcher,arg_def);
    return *this;
  }

  template <typename T, typename... Matchers, typename... Props>
  parser& operator()(T* x,
    const std::tuple<Matchers...>& matchers,
    std::string descr={}, Props&&... p
  ) {
    static_assert( sizeof...(Matchers) > 0,
      "\033[33mempty tuple in program argument definition\033[0m");
    auto *arg_def = add_arg_def(x,std::move(descr),std::forward<Props>(p)...);
    add_arg_matches(matchers,arg_def,std::index_sequence_for<Matchers...>{});
    return *this;
  }
};

}} // end namespace ivanp

#endif
