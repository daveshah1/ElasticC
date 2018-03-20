#include "Operations.hpp"
#include "BitConstant.hpp"
#include <algorithm>
#include <stdexcept>
namespace ElasticC {
int GetResultWidth(vector<int> inWidths, OperationType oper,
                   vector<BitConstant *> constants) {
  switch (oper) {
  case B_ADD:
    return max(inWidths[0], inWidths[1]) + 1;
  case B_SUB:
    return max(inWidths[0], inWidths[1]) + 1;
  case B_MUL:
    return inWidths[0] + inWidths[1];
  case B_DIV:
    if (constants[1] != nullptr) {
      return max((inWidths[0] - inWidths[1]) + 1, 1);
    } else {
      return inWidths[0];
    }
  case B_MOD:
    return inWidths[1];
  case B_LS:
    if (constants[1] != nullptr) {
      return inWidths[0] + constants[1]->intval();
    } else {
      return max(inWidths[0], inWidths[1]);
    }
  case B_RS:
    if (constants[1] != nullptr) {
      return max(inWidths[0] - constants[1]->intval(), 1);
    } else {
      return max(inWidths[0], inWidths[1]);
    }
  case B_BWOR:
    return max(inWidths[0], inWidths[1]);
  case B_BWAND:
    return max(inWidths[0], inWidths[1]);
  case B_BWXOR:
    return max(inWidths[0], inWidths[1]);
  case B_LOR:
  case B_LAND:
  case B_EQ:
  case B_NEQ:
  case B_GT:
  case B_GTE:
  case B_LT:
  case B_LTE:
  case U_LNOT:
    return 1;
  case U_MINUS:
  case U_BWNOT:
    return inWidths[0];
  default:
    throw new runtime_error("Invalid operation passed to GetResultWidth");
  }
}

const Operation *LookupOperation(OperationType type) {
  for (auto &o : unaryPostfixOperations) {
    if (o.type == type) {
      return &o;
    }
  }
  for (auto &o : unaryPrefixOperations) {
    if (o.type == type) {
      return &o;
    }
  }
  for (auto &o : binaryOperations) {
    if (o.type == type) {
      return &o;
    }
  }
  return nullptr;
}

const vector<Operation> unaryPostfixOperations = {
    {U_POSTINC, "++", 2, 1, false, true}, {U_POSTDEC, "--", 2, 1, false, true}};
// NYI: prefix ++/--, unary -
const vector<Operation> unaryPrefixOperations = {
    {U_BWNOT, "~", 3, 1, true, false}, {U_LNOT, "!", 3, 1, true, false},
    {U_PREINC, "++", 3, 1, true, true}, {U_PREDEC, "--", 3, 1, true, true},
    {U_MINUS, "-", 3, 1, true, false}};
const vector<Operation> binaryOperations = {
    {B_MUL, "*", 5, 2, false, false},     {B_DIV, "/", 5, 2, false, false},
    {B_MOD, "%", 5, 2, false, false},     {B_ADD, "+", 6, 2, false, false},
    {B_SUB, "-", 6, 2, false, false},     {B_LS, "<<", 7, 2, false, false},
    {B_RS, ">>", 7, 2, false, false},     {B_LT, "<", 8, 2, false, false},
    {B_LTE, "<=", 8, 2, false, false},    {B_GT, ">", 8, 2, false, false},
    {B_GTE, ">=", 8, 2, false, false},    {B_EQ, "==", 9, 2, false, false},
    {B_NEQ, "!=", 9, 2, false, false},    {B_BWAND, "&", 10, 2, false, false},
    {B_BWXOR, "^", 11, 2, false, false},  {B_BWOR, "|", 12, 2, false, false},
    {B_LAND, "&&", 13, 2, false, false},  {B_LOR, "||", 14, 2, false, false},
    {B_ASSIGN, "=", 15, 2, true, true},   {B_PLUSEQ, "+=", 15, 2, true, true},
    {B_MINUSEQ, "-=", 15, 2, true, true}, {B_MULEQ, "*=", 15, 2, true, true},
    {B_DIVEQ, "/=", 15, 2, true, true},   {B_MODEQ, "%=", 15, 2, true, true},
    {B_LSEQ, "<<=", 15, 2, true, true},   {B_RSEQ, ">>=", 15, 2, true, true},
    {B_ANDEQ, "&=", 15, 2, true, true},   {B_XOREQ, "^=", 15, 2, true, true},
    {B_OREQ, "|=", 15, 2, true, true}

};
}
