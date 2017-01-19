#pragma once
#include "BitConstant.hpp"
#include "DataTypeSpecifier.hpp"
#include "DataTypes.hpp"
#include "Operations.hpp"
#include "ParserCore.hpp"
#include "ParserStatements.hpp"
#include <utility>
#include <vector>
namespace RapidHLS {
namespace Parser {

class Variable;
class GlobalScope;
namespace Templates {
class TemplateParameter;
}
class Context {
public:
  Context *parentContext = nullptr;
  // Find a variable by name in this context and its parents, throw if not found
  virtual Variable *FindVariable(string name);
  // Return true if a variable already exists in the context or its parents
  virtual bool DoesVariableExist(string name);
  // Return a vector of variables declared in the context (only, excluding
  // parents)
  virtual vector<Variable *> GetDeclaredVariables() = 0;
  // Return a pointer to the global scope
  virtual GlobalScope *GetGlobalScope();
  // Return a list of template parameters defined in the current context only
  virtual vector<Templates::TemplateParameter *> GetDefinedTemplateParameters();
  // Returns true if a template parameter exists
  virtual bool IsTemplateParameter(string name);
  // Attempt to find a template parameter, returning its index in the list and
  // context where it occurs or throwing if not found
  virtual pair<Context *, int> FindTemplateParameter(string name);
};

/*
A block is a collection of statements, and can also be used itself as a
statement.
In C++ terms it is anything inside curly braces
*/
class Block : public Statement, public Context {
public:
  vector<Statement *> content;
  vector<Variable *> GetDeclaredVariables(); // from Context
};

/*
for loop
*/
class ForLoop : public Statement, public Context {
public:
  ForLoop(Statement *_init, Expression *_cond, Expression *_inc,
          Statement *_body, const AttributeSet &attr = AttributeSet());
  Statement *initStatement;
  Expression *condition;
  Expression *incrementer;
  Statement *body;
  vector<Variable *> GetDeclaredVariables(); // from Context
};

/*
while loop
*/
class WhileLoop : public Statement {
public:
  WhileLoop(Expression *_cond, Statement *_body,
            const AttributeSet &attr = AttributeSet());
  Expression *condition;
  Statement *body;
};

/*
if statement
*/
class IfStatement : public Statement {
public:
  IfStatement(Expression *_cond, Statement *_true,
              Statement *_false = NullStatement,
              const AttributeSet &attr = AttributeSet());
  Expression *condition;
  Statement *statementTrue;
  Statement *statementFalse = NullStatement;
};

enum class VariableQualifier { STATIC, CONST, REGISTER };
const map<string, VariableQualifier> VariableQualifierStrings = {
    {"static", VariableQualifier::STATIC},
    {"const", VariableQualifier::CONST},
    {"register", VariableQualifier::REGISTER}};
// At the point of parsing variables are fairly simple things
class Variable {
public:
  AttributeSet attributes;
  Context *parentContext;
  DataTypeSpecifier *type;
  string name;
  bool isReference = false; // is a reference?
  vector<VariableQualifier> qualifiers;
  Expression *initialiser = NullExpression;

  // todo: clock domain?
};
// User defined struct
class UserStructure : public Context {
public:
  AttributeSet attributes;

  string name;
  vector<Variable *> members;
  vector<Templates::TemplateParameter *> params;
  vector<Templates::TemplateParameter *> GetDefinedTemplateParameters();
  vector<Variable *> GetDeclaredVariables();
};
// A user defined function
class Function : public Context {
public:
  string name;
  AttributeSet attributes;
  DataTypeSpecifier *returnType;
  bool is_void = false;
  // Stores arguments and a boolean indicating whether or not they are to be
  // passed by reference

  vector<pair<Variable *, bool>> arguments;
  vector<Variable *> GetDeclaredVariables();
  vector<Templates::TemplateParameter *> params;
  virtual vector<Templates::TemplateParameter *> GetDefinedTemplateParameters();

  Statement *body;
};
// Hardware block parameters - special I/Os, etc
struct HardwareBlockParams {
  bool has_clock = false;          // Has a clock input (recommeneded...)
  unsigned long clock_freq = 50e6; // clock frequency
  bool has_cken = false;           // Has a clock enable input
  bool has_den = false;            // Has a data enable input
  bool has_den_out = false;        // Has a data enable output
  bool has_sync_rst = false;       // Has a sync reset input
};

// A top level design block
class HardwareBlock : public Context {
public:
  string name;
  AttributeSet attributes;
  vector<Variable *> inputs;
  vector<Variable *> outputs;
  vector<Variable *> GetDeclaredVariables();
  Statement *body;
  HardwareBlockParams params;
};
// This is the global scope, representing an entire parsed source file
class GlobalScope : public Context {
public:
  vector<Parser::Statement *>
      statements; // these should only be variable declarations
  vector<UserStructure *> structures;
  vector<Function *> functions;
  vector<HardwareBlock *> blocks;

  vector<Variable *> GetDeclaredVariables(); // from Context
};
}
}
