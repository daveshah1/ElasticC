#pragma once
#include "Attributes.hpp"
#include "BitConstant.hpp"
#include "DataTypes.hpp"
#include "Operations.hpp"
#include <vector>
namespace ElasticC {
namespace Parser {

namespace Templates {
class TemplateParameter;
}
class Context;
class Variable;
class Function;
/*
This is a fundamental statement. It could be an operation; or something more
advanced like an if statement
*/
class Statement {
public:
  Statement();
  Statement(const AttributeSet &attr);
  int codeLine = -1; // line where statement occurs, for debugging purposes
  AttributeSet attributes;
  // return variables declared by the statement (NOTE: not in the statement)
  virtual vector<Variable *> GetVariableDeclarations();
};

/*
Does nothing, used to avoid nullptrs for optional statements
As a result is a singleton
*/
class NullStatement_class : public Statement {};
extern NullStatement_class *NullStatement;

/*
An expression could be a basic operation; a variable; a literal or a function
call
*/
class Expression : public Statement {};

/*
Does nothing (returns void?), used to avoid nullptrs for optional statements
As a result is a singleton
*/
class NullExpression_class : public Expression {};
extern NullExpression_class *NullExpression;
/*
A basic operation
*/
class BasicOperation : public Expression {
public:
  BasicOperation(OperationType _type, const vector<Expression *> &_operands);
  OperationType operType;
  vector<Expression *> operands;
};

/*
Parser literal - at the moment only numeric literals are considered
*/
class Literal : public Expression {
public:
  Literal(BitConstant _val);
  BitConstant value;
};

/*
Variable token
*/
class VariableToken : public Expression {
public:
  VariableToken(Variable *_var);
  Variable *var;
};

/*
Array subscripting (operator[])
*/
class ArraySubscript : public Expression {
public:
  ArraySubscript(Expression *_base, const vector<Expression *> &_index);
  Expression *base;
  vector<Expression *> index;
};

/*
Structure member access (operator.)
*/
class MemberAccess : public Expression {
public:
  MemberAccess(Expression *_base, string _mem);
  Expression *base;
  string memberName;
};

/*
Function call
*/
class FunctionCall : public Expression {
public:
  FunctionCall(Function *_func, const vector<Expression *> &_operands);
  Function *func;
  vector<Expression *> operands;
  vector<Templates::TemplateParameter *> params;
};

/*
Initialiser list (only allowed in specific contexts)
*/
class InitialiserList : public Expression {
public:
  InitialiserList(const vector<Expression *> &_values);
  vector<Expression *> values;
};

/*
"Builtin" expression
*/
enum class BuiltinType {
  SIZEOF,  // C-style sizeof (in bytes)
  WIDTHOF, // width in bits
  LENGTH,  // array length
  MIN,     // min value of scalar
  MAX,     // max value of scalar
};

extern const map<string, BuiltinType> BuiltinTokens;

class Builtin : public Expression {
public:
  Builtin(BuiltinType _type, Expression *_operand);
  BuiltinType type;
  Expression *operand;
};

/*
Template parameter token, given as an index into the current set of template
parameters
*/
class TemplateParamToken : public Expression {
public:
  TemplateParamToken(Context *_context, int _index);
  Context *pcontext; // context where the parameter occurs
  int index;
};

/*
Variable or constant declaration
*/
class VariableDeclaration : public Statement {
public:
  VariableDeclaration(const vector<Variable *> &_declVar,
                      const AttributeSet &attr = AttributeSet());
  vector<Variable *> declaredVariables;
  vector<Variable *> GetVariableDeclarations();
};

/*
return statement (valid in functions only)
*/
class ReturnStatement : public Statement {
public:
  ReturnStatement(Expression *_retval);
  Expression *returnValue = NullExpression;
};
}
}
