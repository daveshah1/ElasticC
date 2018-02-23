#pragma once
#include "Operations.hpp"
#include <string>
#include <vector>
using namespace std;
namespace ElasticC {
/*
This is intended to store the value of an arbitrary length constant
*/
class BitConstant {
public:
  BitConstant();
  BitConstant(const BitConstant &other);
  BitConstant(string constval);
  BitConstant(int intval);
  BitConstant(int intval, int width);
  BitConstant(unsigned long long intval);
  vector<bool> bits; // note bits[0] is LSB
  bool is_signed;
  void trim();        // remove redundant bits
  int intval() const; // get value as integer
  string to_string() const;
  BitConstant cast(int newsize, bool newsigned) const;
};

// These functions implement various specific operations on BitConstants
BitConstant InvertBits(BitConstant a);
BitConstant AddBits(BitConstant a, BitConstant b, bool cin = false);
BitConstant SubtractBits(BitConstant a, BitConstant b);
BitConstant MultiplyBits(BitConstant a, BitConstant b);
BitConstant LeftShiftBits(BitConstant val, BitConstant amt);
BitConstant RightShiftBits(BitConstant val, BitConstant amt);
BitConstant AreBitsEqual(BitConstant a, BitConstant b);
BitConstant IsLessThan(BitConstant a, BitConstant b);
BitConstant BitwiseBitOperation(BitConstant a, BitConstant b,
                                OperationType oper);
BitConstant LogicalBitOperation(const vector<BitConstant> &operands,
                                OperationType oper);

// Perform an operation on two constants, returning the result as a constant
// This is designed to work for arbitrary length constants
BitConstant PerformConstOperation(vector<BitConstant> operands,
                                  OperationType oper);
};
