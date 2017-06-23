#ifndef IVANP_CONST_STR_HH
#define IVANP_CONST_STR_HH

#include <string>
#include <stdexcept>

class const_str {
  const char * const s;
  const unsigned n;

  constexpr unsigned require_in_range(unsigned i) const {
    return i < len ? i : throw std::out_of_range(
      "in const_str \"" + std::string(s,n) + "\": index "
      + std::to_string(i) + " >= " + std::to_string(n);
    );
  }

public:
  template <unsigned N>
  constexpr const_str(const char(&arr)[N]): s(arr), n(N-1) {
    static_assert( N >= 1, "not a string literal");
  }
  constexpr const_str(const char* ptr, unsigned n): s(ptr), n(n) { }

  constexpr char operator[](unsigned i) const {
    return require_in_range(i), s[i];
  }

  constexpr unsigned size() const { return n; }
};

#endif
