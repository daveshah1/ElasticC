#pragma once
#include "Attributes.hpp"
#include "ParserCore.hpp"
#include "ParserStatements.hpp"
#include "ParserStructures.hpp"
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;
namespace ElasticC {
namespace Parser {

namespace Templates {
class TemplateParameter;
}

// The ElasticC parser itself
class ECCParser {
public:
  ECCParser(ParserState &_code, GlobalScope &_gs);
  ParserState code;
  GlobalScope &gs;
  // Parse the entire global scope
  void ParseAll();

  // parse a data type specifier looking at declared structures
  bool IsDataTypeKeyword(string keyword);
  bool TryParseDataType(DataTypeSpecifier *&result);
  DataTypeSpecifier *ParseDataType(Context *ctx);

  // parse a single statement
  Statement *ParseStatement(Context *ctx);

  // parse the content of a block
  Block *ParseBlockContent(Context *ctx);

  // parse an expression until one of a set of terminators is reached
  // note that a 'close bracket' terminator will only stop when the last of
  // nested brackets is reached. does not consume terminator
  Expression *ParseExpression(vector<char> terminators, Context *ctx);

  // parse a structure definiton and add it to the global scope
  void ParseStructureDefinition(const AttributeSet &currentAttr,
                                vector<Templates::TemplateParameter *> templ);

  // parse a variable declaration
  // setting oneOnly to true is used when we only want to pick up a
  // single variable (e.g. block IO and function argument lists)
  // isRef is set to true if is a reference
  VariableDeclaration *ParseVariableDeclaration(Context *ctx,
                                                const AttributeSet &currentAttr,
                                                bool &isRef,
                                                bool oneOnly = false);

  // parse a hardware block and its header
  void ParseHWBlock(const AttributeSet &currentAttr);

  // parse a function and its header
  void ParseFunction(const AttributeSet &currentAttr,
                     vector<Templates::TemplateParameter *> templ);

  // parse any attributes
  AttributeSet ParseAttributes();

  // parse a preprocessor definition
  void ParsePreprocessorDef();

  // Include a file following sensible ElasticC semantics not dated C semantics
  // (ie header files are treated as modules rather than just included as text)
  // If quiet is true reporting is reduced, used for internal header files
  void IncludeFile(string fileName, bool systemOnly = false,
                   bool quiet = false);

  // helper function to parse a constant expression and return its value
  BitConstant ParseConstExpr(vector<char> terminators, Context *ctx);

  // Parse the template parameter definition for a function or struct
  vector<Templates::TemplateParameter *> ParseTemplateDefinition(Context *ctx);

  // Parse a variable expression, including array subscripting and structure
  // member access
  Expression *ParseVarExpression(Context *ctx);

  // Determine whether or not a given token is the name of a function
  bool IsFunction(string token);

  // list of pragma statements
  vector<string> pragmas;

private:
  map<string, DataTypeSpecifier *> typedefs;

  // Handle a potentially multi-dimensional (or non-existent, in which case
  // baseType is returned intact) array type specifier
  DataTypeSpecifier *HandleArraySpecifier(DataTypeSpecifier *baseType,
                                          Context *ctx);

  // Parse an argument list, for a function or HardwareBlock
  // Returns a pair specifying varibale for agument and whether or not by
  // reference
  // This can also look for 'special' arguments (e.g clock); which can
  // optionally take parameters in the template style
  vector<pair<Variable *, bool>> ParseArgumentList(
      Context *ctx,
      map<string, vector<Templates::TemplateParameter *>> &specialParams,
      map<string, bool> &specialParamFound);

  enum class OpStackItemType { L_PAREN, R_PAREN, OPER };

  // Used in our implementation of the shunting yard algorithm
  struct OperationStackItem {
    OperationStackItem(OpStackItemType _type);
    OperationStackItem(OpStackItemType _type, OperationType _oper);

    OpStackItemType type;
    OperationType oper;
  };

  // Pop an item from the operation stack and apply it to the parse stack
  void ApplyFromOpStack(stack<OperationStackItem> &opStack,
                        stack<Expression *> &parseStack);

  // Parse a comma separated list of expressions, ending with a given terminator
  // The terminator is not pulled from the stream
  vector<Expression *> ParseExpressionList(Context *ctx, char term);

  // unary prefix token strings, and matching array containing operation type
  vector<string> unaryPrefixOperTokens;
  vector<OperationType> unaryPrefixOperTypes;
  // binary and unary postfix token strings, and matching array containg
  // operation type
  vector<string> binaryAndPostfixOperTokens;
  vector<OperationType> binaryAndPostfixOperTypes;
};

class parse_error : public runtime_error {
  using runtime_error::runtime_error;
};
}
}
