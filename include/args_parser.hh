#ifndef IVANP_ARGS_PARSER_HH
#define IVANP_ARGS_PARSER_HH

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <type_traits>

#include "utility.hh"
#include "arg_match.hh"
#include "arg_def.hh"

#include "type.hh"

namespace ivanp { namespace args {

class parser {
  std::vector<std::unique_ptr<detail::arg_def_base>> arg_defs;
  std::array<std::vector<std::pair<
    std::unique_ptr<const detail::arg_match_base>,
    const detail::arg_def_base*
  >>,3> matchers;

  template <typename T, typename... Props>
  inline auto* make_arg_def(T* x, std::string&& descr, Props&&... p) {
    using props_types = std::tuple<std::decay_t<Props>...>;
    const auto props  = std::forward_as_tuple(std::forward<Props>(p)...);

#define UNIQUE_ASSERT(NAME) \
    using NAME##_i = get_indices_of_t< \
      ::ivanp::args::is_##NAME, props_types>; \
    static_assert( NAME##_i::size() <= 1, \
      "\033[33mrepeated \"" #NAME "\" in argument definition\033[0m");

    UNIQUE_ASSERT(named)
    UNIQUE_ASSERT(pos)
    UNIQUE_ASSERT(req)
    UNIQUE_ASSERT(multi)
    UNIQUE_ASSERT(tag)

#undef UNIQUE_ASSERT

    using seq = seq_join_t< named_i, pos_i, req_i, multi_i, tag_i >;

    static_assert( seq::size() == sizeof...(Props),
      "\033[33munrecognized option in argument definition\033[0m");

    auto *arg_def = detail::make_arg_def(x, std::move(descr), props, seq{});
    arg_defs.emplace_back(arg_def);

    type_size<std::decay_t<decltype(*arg_def)>>();

    return arg_def;
  }

public:
  void parse(int argc, char const * const * argv);
  // void help(); // FIXME

  template <typename T, typename... Props>
  parser& operator()(T* x,
    std::initializer_list<const char*> matchers, std::string descr={},
    Props&&... p
  ) {
    auto *arg_def = make_arg_def(x,std::move(descr),std::forward<Props>(p)...);
    for (const char* x : matchers) {
      // arg_def->name += to_str_if_can(x);
      // if (&x != &*matchers.end()) arg_def->name += ", ";
      // TODO: compute names only for help
      auto&& m = detail::make_arg_match(x);
      // TODO: find out what binding to auto&& really does
      this->matchers[m.second].emplace_back(std::move(m.first),arg_def);
    }
    return *this;
  }

  template <typename T, typename Matcher, typename... Props>
  parser& operator()(T* x,
    Matcher&& matcher, std::string descr={}, Props&&... p
  ) {
    auto *arg_def = make_arg_def(x,std::move(descr),std::forward<Props>(p)...);
    auto&& m = detail::make_arg_match(std::forward<Matcher>(matcher));
    matchers[m.second].emplace_back(std::move(m.first),arg_def);
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
