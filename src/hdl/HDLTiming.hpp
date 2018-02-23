#pragma once
#include <string>
#include <vector>
using namespace std;

namespace ElasticC {
namespace HDLGen {
/*
This class provides a generic interface for timing delay values. It is used to
represent both timing delays (clock-to-valid) using the double specialisation
and pipeline latencies (clock cycles from input to valid) using the integer
specialisation
*/
class HDLSignal;

template <typename T> class HDLTimingValue {
public:
  // Create a don't care constraint
  inline HDLTimingValue() : domain(nullptr){};
  // Create a numeric constraint
  inline HDLTimingValue(HDLSignal *_domain, T _value)
      : domain(_domain), value(_value){};

  HDLSignal *domain = nullptr;
  T value;
};

template <typename T>
inline bool operator==(const HDLTimingValue<T> &a, const HDLTimingValue<T> &b) {
  if ((a.domain == nullptr) || (b.domain == nullptr)) {
    // don't care constraints always equal
    return true;
  } else if (a.domain != b.domain) {
    // mismatched domains are a don't care so always equal
    return true;
  } else {
    return a.value == b.value;
  }
};

template <typename T>
inline bool operator<(const HDLTimingValue<T> &a, const HDLTimingValue<T> &b) {
  if ((a.domain == nullptr) || (b.domain == nullptr)) {
    // don't care constraints always count as less
    return true;
  } else if (a.domain != b.domain) {
    // mismatched domains are a don't care so count as less
    return true;
  } else {
    return a.value < b.value;
  }
};

template <typename T>
inline bool operator>(const HDLTimingValue<T> &a, const HDLTimingValue<T> &b) {
  if ((a.domain == nullptr) || (b.domain == nullptr)) {
    // don't care constraints don't count as more
    return false;
  } else if (a.domain != b.domain) {
    // mismatched domains are a don't care so don't count as more
    return false;
  } else {
    return a.value > b.value;
  }
};

template <typename T>
inline HDLTimingValue<T> operator+(const HDLTimingValue<T> &a, T b) {
  if (a.domain == nullptr)
    return HDLTimingValue<T>();
  else
    return HDLTimingValue<T>(a.domain, a.value + b);
}
}
}
