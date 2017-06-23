#ifndef IVANP_STRING_1_HH
#define IVANP_STRING_1_HH

#include <string>
#include <sstream>
#include <type_traits>

#include "detect.hh"

template <typename T>
using det_ss = decltype(std::stringstream{}<<std::declval<T>());

template <typename T>
std::enable_if_t<is_detected<det_ss,T>::value,std::string>
to_str_if_can(const T& x) {
  std::stringstream ss;
  ss << x;
  return ss.str();
}

template <typename T>
std::enable_if_t<!is_detected<det_ss,T>::value,std::string>
inline to_str_if_can(const T& x) { return {}; }

#endif
