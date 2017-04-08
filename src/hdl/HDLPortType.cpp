#include "HDLPortType.hpp"
#include <algorithm>
#include <sstream>
using namespace std;
namespace RapidHLS {
namespace HDLGen {

bool HDLPortType::IsSigned() const { return false; }

string HDLPortType::VHDLCastFrom(const HDLPortType *other,
                                 const string &value) const {
  return value;
}

string LogicSignalPortType::GetVHDLType() const { return "std_logic"; }

int LogicSignalPortType::GetWidth() const { return 1; }

string LogicSignalPortType::VHDLCastFrom(const HDLPortType *other,
                                         const string &value) const {
  if (dynamic_cast<const LogicSignalPortType *>(other) != nullptr) {
    return value;
  } else {
    return value + "(0)";
  }
};

LogicVectorPortType::LogicVectorPortType(int _width) : width(_width){};

string LogicVectorPortType::GetVHDLType() const {
  return "std_logic_vector(" + to_string(width - 1) + " downto 0)";
};

int LogicVectorPortType::GetWidth() const { return width; };

static inline string zeros(int n) { return "\"" + string(n, '0') + "\""; }

string LogicVectorPortType::VHDLCastFrom(const HDLPortType *other,
                                         const string &value) const {
  if (dynamic_cast<const LogicSignalPortType *>(other) != nullptr) {
    return "std_logic_vector(" + zeros(width - 1) + " & " + value + ")";
  } else if (dynamic_cast<const NumericPortType *>(other) != nullptr) {
    return "std_logic_vector(resize(" + value + ", " + to_string(width) + "))";
  } else {
    ostringstream out;
    if (width > other->GetWidth()) {
      out << zeros(width - other->GetWidth()) << " & ";
    }
    if (dynamic_cast<const LogicVectorPortType *>(other) != nullptr) {
      out << value;
    } else {
      out << "std_logic_vector(" << value << ")";
    }
    if (width < other->GetWidth()) {
      out << "(" << (width - 1) << " downto 0)";
    }
    return out.str();
  }
};

NumericPortType::NumericPortType(int _width, bool _signed)
    : width(_width), is_signed(_signed){};

string NumericPortType::GetVHDLType() const {
  return string(is_signed ? "signed(" : "unsigned(") + to_string(width - 1) +
         " downto 0)";
}

int NumericPortType::GetWidth() const { return width; }

string NumericPortType::VHDLCastFrom(const HDLPortType *other,
                                     const string &value) const {
  string curr_value = "";
  if (dynamic_cast<const NumericPortType *>(other) == nullptr) {
    // Go via a std_logic_vector first
    curr_value =
        LogicVectorPortType(other->GetWidth()).VHDLCastFrom(other, value);
  } else {
    curr_value = value;
  }

  curr_value = string(is_signed ? "signed(" : "unsigned(") + curr_value + ")";

  if (other->GetWidth() != width) {
    curr_value = "resize(" + curr_value + ", " + to_string(width) + ")";
  }
  return curr_value;
}
}
}
