#ifndef IVANP_STRING_1_HH
#define IVANP_STRING_1_HH

#include <string>
#include <sstream>
#include <utility>
#include <type_traits>

#include "detect.hh"

namespace ivanp {
namespace detail {

template <typename S, typename T>
inline void cat_impl(S& s, const T& t) { s << t; }

template <typename S, typename T, typename... TT>
inline void cat_impl(S& s, const T& t, const TT&... tt) {
  s << t;
  cat_impl(s,tt...);
}

template <typename T>
using det_ss = decltype(std::ostringstream{}<<std::declval<T>());

} // detail

template <typename... TT>
inline std::string cat(const TT&... tt) {
  std::ostringstream ss;
  detail::cat_impl(ss,tt...);
  return ss.str();
}

template <typename T>
std::enable_if_t<is_detected<detail::det_ss,T>::value,std::string>
to_str_if_can(const T& x) {
  std::ostringstream ss;
  ss << x;
  return ss.str();
}

template <typename T>
std::enable_if_t<!is_detected<detail::det_ss,T>::value,std::string>
inline to_str_if_can(const T& x) { return {}; }

} // ivanp

#endif
