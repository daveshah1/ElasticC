#pragma once
#include "DataTypes.hpp"
#include <string>
#include <vector>
using namespace std;
namespace RapidHLS {
class BitConstant;
enum OperationType {
  B_ADD, // arithmetic
  B_SUB,
  B_MUL,
  B_DIV,
  B_MOD,
  B_LS, // shift
  B_RS,
  B_BWOR, // bitwise
  B_BWAND,
  B_BWXOR,
  B_LOR, // logical
  B_LAND,
  B_EQ, // comparison
  B_NEQ,
  B_GT,
  B_GTE,
  B_LT,
  B_LTE,
  B_ASSIGN, // basic assignment
  B_PLUSEQ, // combined assignment
  B_MINUSEQ,
  B_MULEQ,
  B_DIVEQ,
  B_MODEQ,
  B_OREQ,
  B_ANDEQ,
  B_XOREQ,
  B_LSEQ,
  B_RSEQ,
  U_MINUS,   // unary minus
  U_POSTINC, // unary pre/post inc/dec
  U_PREINC,
  U_POSTDEC,
  U_PREDEC,
  U_BWNOT, // unary bitwise not
  U_LNOT   // unary logical not
};

struct Operation {
  OperationType type;
  string token;
  int precedence;
  int num_params;
  bool right_associative;
  bool is_assignment;
};

extern const vector<Operation> unaryPostfixOperations;
extern const vector<Operation> unaryPrefixOperations;
extern const vector<Operation> binaryOperations;

// Returns the width of the result of performing an operation on two values
// constants[i] is nullptr if operand i is not a constant, or its value
// otherwise
int GetResultWidth(vector<int> inWidths, OperationType oper,
                   vector<BitConstant *> constants);
// Returns a pointer to an operation given the OperationType enum value
const Operation *LookupOperation(OperationType type);
}
