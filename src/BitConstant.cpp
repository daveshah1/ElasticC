#include "BitConstant.hpp"
#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <iostream>
using namespace std;

namespace ElasticC {

int getNumericValue(char c) {
  c = toupper(c);
  if ((c >= '0') && (c <= '9')) {
    return c - '0';
  } else if ((c >= 'A') && (c <= 'Z')) {
    return (c - 'A') + 10;
  } else {
    return -1;
  }
}

BitConstant::BitConstant() {}

BitConstant::BitConstant(const BitConstant &other)
    : bits(other.bits), is_signed(other.is_signed){};

BitConstant::BitConstant(string constval) {
  if (constval.length() > 0) {
    int offset = 0;
    if (constval[0] == '-') {
      is_signed = true;
      offset++;
    } else {
      is_signed = false;
    }
    int base = 10;
    if (constval.size() >= (2 + offset)) {
      if (constval[offset] == '0') {
        offset++;
        if (constval[offset] == 'x') {
          base = 16;
          offset++;
        } else if (constval[offset] == 'b') {
          base = 2;
          offset++;
        } else {
          base = 8;
        }
      }
    }

    // reverse order as last digit has the lowest value
    for (int i = offset; i < constval.size(); i++) {
      int digitVal = getNumericValue(constval[i]);

      if ((digitVal >= base) || (digitVal == -1)) {
        throw runtime_error(string("Unexpected digit ") + constval[i] +
                            " in numeric constant");
      }
      bits = PerformConstOperation(
                 vector<BitConstant>{BitConstant(base), *this}, B_MUL)
                 .bits; // multiply by base to shift existing value
      bits = PerformConstOperation(
                 vector<BitConstant>{BitConstant(digitVal), *this},
                 B_ADD)
                 .bits; // add value of current digit
    }
    if (bits.size() == 0) {
      bits.push_back(false); // force at least one bit
    }
    // negate, 2's complement
    if (is_signed) {
      bits.push_back(false);
      bits = PerformConstOperation(vector<BitConstant>{BitConstant(0), *this},
                                   B_SUB)
                 .bits;
    };

  } else {
    is_signed = false;
    bits.push_back(false);
  }

  trim();
}

BitConstant::BitConstant(int intval) {
  bool neg = false;
  if (intval < 0) {
    neg = true;
    intval = -intval;
  }
  while (intval > 0) {
    bits.push_back(~~(intval & 0x1));
    intval >>= 1;
  }

  if (neg) {
    // negate, 2's complement
    bits.push_back(false);
    bits =
        PerformConstOperation(vector<BitConstant>{BitConstant(0), *this}, B_SUB)
            .bits;
    is_signed = true;
  } else {
    is_signed = false;
  }

  if (bits.size() == 0)
    bits.push_back(false);
  trim();
}

BitConstant::BitConstant(int intval, int width) {
  bool neg = false;
  if (intval < 0) {
    neg = true;
    intval = -intval;
  }
  while (intval > 0) {
    bits.push_back(~~(intval & 0x1));
    intval >>= 1;
  }

  if (neg) {
    // negate, 2's complement
    bits.push_back(false);
    bits =
        PerformConstOperation(vector<BitConstant>{BitConstant(0), *this}, B_SUB)
            .bits;
    is_signed = true;
  } else {
    is_signed = false;
  }

  if (bits.size() == 0)
    bits.push_back(false);

  while (bits.size() > width) {
    bits.pop_back();
  }

  while (bits.size() < width) {
    if (is_signed) {
      bits.push_back(bits.back());
    } else {
      bits.push_back(false);
    }
  }
}

BitConstant::BitConstant(unsigned long long intval) {
  is_signed = false;
  while (intval > 0) {
    bits.push_back(~~(intval & 0x1));
    intval >>= 1;
  }
  if (bits.size() == 0)
    bits.push_back(false);
  trim();
}

void BitConstant::trim() {
  // signed numbers require a bit of care
  if (is_signed) {
    // remove all the sign bits
    if (bits.size() > 0) {
      bool sign_bit = bits.back();
      while ((bits.size() > 0) && (bits.back() == sign_bit)) {
        bits.erase(bits.end() - 1);
      }
      // restore exactly one sign bit
      bits.push_back(sign_bit);
    }
  } else {
    // for unsigned numbers, just trim off
    // Ensure length does not fall below 1
    while ((bits.size() > 1) && (bits.back() == false)) {
      bits.erase(bits.end() - 1);
    }
  }
}

string BitConstant::to_string() const {
  string out = "\"";
  for (int i = bits.size() - 1; i >= 0; i--) {
    if (bits[i]) {
      out += "1";
    } else {
      out += "0";
    }
  }
  return out + "\"";
}

int BitConstant::intval() const {
  int ival = 0;
  bool sign = false;
  int start = bits.size() - 1;
  if (is_signed) {
    sign = bits.back();
    start--;
  }

  for (int i = start; i >= 0; i--) {
    ival <<= 1;
    if (bits[i] != sign) { // the != sign deals with 2's complement inversion
      ival++;
    }
  }

  if (sign) {
    ival += 1;
    ival = -ival;
  }

  return ival;
}

BitConstant BitConstant::cast(int newsize, bool newsigned) const {
  BitConstant casted;
  casted.is_signed = newsigned;
  for (int i = 0; i < newsize; i++) {
    if (i < bits.size()) {
      casted.bits.push_back(bits[i]);
    } else if ((is_signed) && (bits.size() > 0)) {
      casted.bits.push_back(bits.back()); // sign extend
    } else {
      casted.bits.push_back(false);
    }
  }
  return casted;
}

BitConstant InvertBits(BitConstant a) {
  // Special case: if a is 0 length then the result is a '1' constant
  if (a.bits.size() == 0) {
    return BitConstant(1);
  } else {
    BitConstant result;
    result.is_signed = a.is_signed;
    for (int i = 0; i < a.bits.size(); i++) {
      result.bits.push_back(!a.bits[i]);
    }
    return result;
  }
}

BitConstant AddBits(BitConstant a, BitConstant b, bool cin) {
  BitConstant result;
  result.is_signed = a.is_signed || b.is_signed;
  result.bits.resize(max(a.bits.size(), b.bits.size()) + 1);
  bool carry = cin;
  for (int i = 0; i < result.bits.size(); i++) {
    bool int_a, int_b;
    if (i < a.bits.size())
      int_a = a.bits[i];
    else if (a.is_signed) // sign extend
      int_a = a.bits.back();
    else
      int_a = false;

    if (i < b.bits.size())
      int_b = b.bits[i];
    else if (b.is_signed) // sign extend
      int_b = b.bits.back();
    else
      int_b = false;
    // full adder
    result.bits[i] = (int_a != int_b) != carry;
    carry = (int_a && int_b) || ((int_a != int_b) && carry);
  }
  return result;
}

BitConstant SubtractBits(BitConstant a, BitConstant b) {
  BitConstant temp_b =
      b.cast(max(a.bits.size(), b.bits.size()), a.is_signed | b.is_signed);
  return AddBits(a, InvertBits(temp_b), true);
}

BitConstant MultiplyBits(BitConstant a, BitConstant b) {
  BitConstant result;
  result.is_signed = a.is_signed || b.is_signed;
  result.bits.resize(a.bits.size() + b.bits.size(), false);
  BitConstant temp_a = a;
  for (int i = 0; i < b.bits.size(); i++) {
    if (b.bits[i]) {
      result = AddBits(result, temp_a);
    }
    temp_a = AddBits(temp_a, temp_a);
  }
  return result;
}

BitConstant BitwiseBitOperation(BitConstant a, BitConstant b,
                                OperationType oper) {
  BitConstant result;
  result.is_signed = a.is_signed || b.is_signed;
  result.bits.resize(max(a.bits.size(), b.bits.size()));
  for (int i = 0; i < result.bits.size(); i++) {
    bool int_a, int_b;
    if (i < a.bits.size())
      int_a = a.bits[i];
    else if (a.is_signed) // sign extend
      int_a = a.bits.back();
    else
      int_a = false;

    if (i < b.bits.size())
      int_b = b.bits[i];
    else if (b.is_signed) // sign extend
      int_b = b.bits.back();
    else
      int_b = false;

    bool bitresult = false;
    switch (oper) {
    case B_BWOR:
      bitresult = int_a || int_b;
      break;
    case B_BWAND:
      bitresult = int_a && int_b;
      break;
    case B_BWXOR:
      bitresult = (int_a != int_b);
      break;
    default:
      break;
    }
    result.bits[i] = bitresult;
  }
  return result;
}

BitConstant IsLessThan(BitConstant a, BitConstant b) {
  if (a.is_signed) {
    if (b.is_signed) {
      if ((a.bits.back()) && (!b.bits.back())) // a -ve, b +ve
        return BitConstant(1);
      if ((!a.bits.back()) && (b.bits.back())) // a +ve, b -ve
        return BitConstant(0);
    } else {
      if (a.bits.back()) // a -ve, b +ve unsigned
        return BitConstant(1);
    }
  } else if (b.is_signed) {
    if (b.bits.back()) // a +ve unsigned, b -ve
      return BitConstant(0);
  }
  int comp_size = max(a.bits.size(), b.bits.size());
  for (int i = comp_size - 1; i >= 0; i--) {
    bool int_a, int_b;
    if (i < a.bits.size())
      int_a = a.bits[i];
    else if (a.is_signed) // sign extend
      int_a = a.bits.back();
    else
      int_a = false;

    if (i < b.bits.size())
      int_b = b.bits[i];
    else if (b.is_signed) // sign extend
      int_b = b.bits.back();
    else
      int_b = false;

    if ((int_a) && (!int_b))
      return BitConstant(0); // a=1, b=0 : >
    if ((!int_a) && (int_b))
      return BitConstant(1); // a=0, b=1 : <

    // otherwise equal so far, continue
  };
  return BitConstant(0); // they're equal
}

BitConstant LogicalBitOperation(const vector<BitConstant> &operands,
                                OperationType oper) {
  vector<bool> booleanOperands;
  for (auto operand : operands) {
    bool lval = false;
    for (auto bit : operand.bits) {
      if (bit) {
        lval = true;
      }
    }
    booleanOperands.push_back(lval);
  }
  switch (oper) {
  case B_LOR:
    return BitConstant(booleanOperands[0] || booleanOperands[1]);
  case B_LAND:
    return BitConstant(booleanOperands[0] || booleanOperands[1]);
  case U_LNOT:
    return BitConstant(!booleanOperands[0]);
  default:
    return BitConstant(0);
  }
}

BitConstant LeftShiftBits(BitConstant val, BitConstant amt) {
  BitConstant result = val;
  result.bits.insert(result.bits.begin(), amt.intval(), false);
  return result;
}

BitConstant RightShiftBits(BitConstant val, BitConstant amt) {
  BitConstant result = val;
  if (amt.intval() > val.bits.size()) {
    result = BitConstant(0);
  } else if (amt.intval() > 0) {
    result.bits.erase(result.bits.begin(), result.bits.begin() + amt.intval());
  }
  return result;
}

BitConstant AreBitsEqual(BitConstant a, BitConstant b) {
  BitConstant result;
  bool isEqual = true;
  int msize = max(a.bits.size(), b.bits.size());
  for (int i = 0; i < msize; i++) {
    bool int_a, int_b;
    if (i < a.bits.size())
      int_a = a.bits[i];
    else if (a.is_signed) // sign extend
      int_a = a.bits.back();
    else
      int_a = false;

    if (i < b.bits.size())
      int_b = b.bits[i];
    else if (b.is_signed) // sign extend
      int_b = b.bits.back();
    else
      int_b = false;

    if (int_a != int_b) {
      isEqual = false;
      break;
    }
  }
  return BitConstant(isEqual);
}
// also need constant compare operations

BitConstant PerformConstOperation(vector<BitConstant> operands,
                                  OperationType oper) {
  BitConstant result;
  switch (oper) {
  case B_ADD:
    result = AddBits(operands[0], operands[1]);
    break;
  case B_SUB:
    result = SubtractBits(operands[0], operands[1]);
    break;
  case B_MUL:
    result = MultiplyBits(operands[0], operands[1]);
    break;
  // no divide/modulo as these are eliminated and converted to multiply
  case B_LS:
    result = LeftShiftBits(operands[0], operands[1]);
    break;
  case B_RS:
    result = RightShiftBits(operands[0], operands[1]);
    break;
  case B_BWOR:
  case B_BWAND:
  case B_BWXOR:
    result = BitwiseBitOperation(operands[0], operands[1], oper);
    break;
  case B_LOR:
  case B_LAND:
  case U_LNOT:
    result = LogicalBitOperation(operands, oper);
    break;
  case B_EQ:
    result = AreBitsEqual(operands[0], operands[1]);
    break;
  case B_NEQ:
    result = InvertBits(AreBitsEqual(operands[0], operands[1]));
    break;
  case B_LT:
    result = IsLessThan(operands[0], operands[1]);
    break;
  case B_GTE:
    result = InvertBits(IsLessThan(operands[0], operands[1]));
    break;
  case B_LTE:
    result = LogicalBitOperation(
        vector<BitConstant>{IsLessThan(operands[0], operands[1]),
                            AreBitsEqual(operands[0], operands[1])},
        B_LOR);
    break;
  case B_GT:
    result = InvertBits(LogicalBitOperation(
        vector<BitConstant>{IsLessThan(operands[0], operands[1]),
                            AreBitsEqual(operands[0], operands[1])},
        B_LOR));
    break;
  default:
    throw runtime_error("cannot determine const result for operator" +
                        LookupOperation(oper)->token);
    break;
  }
  result.trim();
  return result;
}
}
