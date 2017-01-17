#pragma once
#include "EvalObject.hpp"
#include "EvaluatorState.hpp"
#include "ParserStatements.hpp"
#include "ParserStructures.hpp"
#include <map>
#include <stack>
#include <string>
#include <utility>
#include <vector>
using namespace std;
namespace RapidHLS {
// This evaluates the parsed source into something that can be converted to
// hardware (typically this process consists of timing analysis, pipelining if
// needed and code [VHDL/POLYMER] generation)

// It is left as a generic interface so alternative 'strategies' can be
// implemented in the future, such as a proper multicycle tool or, more useful
// for my planned uses, multi-pixel-per-clock systems

class CallStackEntry;
class TemplateParamContext;
class Evaluator {
public:
  Evaluator(Parser::GlobalScope *_gs);
  // Evaluate the parsed source for a block, returning the final Evaluator
  // state
  virtual void EvaluateBlock(Parser::HardwareBlock *block) = 0;
  virtual void EvaluateStatement(Parser::Statement *stmt) = 0;
  virtual EvalObject *EvaluateExpression(Parser::Expression *expr) = 0;
  virtual EvalObject *EvaluateInitialiser(Parser::Expression *expr,
                                          Parser::DataTypeSpecifier *type);

  virtual void SetVariableValue(EvaluatorVariable *var, EvalObject *value) = 0;
  virtual EvalObject *GetVariableValue(EvaluatorVariable *var) = 0;
  // Add a completely new variable
  virtual void AddVariable(EvaluatorVariable *var);
  // Add a variable paired to one in the parser
  virtual void AddVariable(EvaluatorVariable *var, Parser::Variable *orig);
  // Add a variable given the parser variable
  virtual EvaluatorVariable *AddVariable(Parser::Variable *orig,
                                         bool is_block_input = false,
                                         bool is_block_output = false);
  // Return a list of all variables
  virtual vector<EvaluatorVariable *> GetAllVariables();
  // Return the evaluator variable corresponding to a parser variable
  virtual EvaluatorVariable *GetVariableByParserVar(Parser::Variable *orig);
  // Process a function call
  virtual EvalObject *ProcessFunctionCall(
      Parser::Function *func, vector<EvalObject *> arguments,
      vector<Parser::Templates::TemplateParameter *> templateParams);

protected:
  vector<EvaluatorVariable *> allVariables;
  map<Parser::Variable *, EvaluatorVariable *> parserVariables;
  Parser::GlobalScope *gs;
  stack<CallStackEntry *> callStack;

  TemplateParamContext *tpContext;
};

// Context used to resolve template parameters
class TemplateParamContext {
public:
  TemplateParamContext *parent = nullptr;
  Parser::Context *pContext = nullptr;
  vector<Parser::Templates::TemplateParameter *> templateParams;

  BitConstant GetNumericParameter(Evaluator *eval, Parser::Context *origCtx,
                                  int index);
  Parser::DataTypeSpecifier *
  GetTypeParameter(Evaluator *eval, Parser::Context *origCtx, int index);
};

// This is used by the evaluator to handle function calls
class CallStackEntry {
public:
  // Pointer to the called function
  Parser::Function *calledFunction;
  // Variable used to store return value
  EvaluatorVariable *returnValue;
  // Template parameters passed to the function
  vector<Parser::Templates::TemplateParameter *> templateParams;
  // In order to support recursive functions in the future; save and restore the
  // parserVariables map so the same parser variable inside the function can
  // refer to different eval variables
  map<Parser::Variable *, EvaluatorVariable *> savedParserVariables;
  // Value of tpContext before function call started
  TemplateParamContext *oldTpContext;
};

class SingleCycleEvaluator : public Evaluator {
public:
  SingleCycleEvaluator(Parser::GlobalScope *_gs);
  virtual void EvaluateBlock(Parser::HardwareBlock *block);
  virtual void AddVariable(EvaluatorVariable *var);
  virtual void SetVariableValue(EvaluatorVariable *var, EvalObject *value);
  virtual EvalObject *GetVariableValue(EvaluatorVariable *var);
  virtual void EvaluateStatement(Parser::Statement *stmt);
  virtual EvalObject *EvaluateExpression(Parser::Expression *expr);

private:
  map<EvaluatorVariable *, EvalObject *> currentVariableValues;
  // Used to keep track of the current if condition. The second boolean
  // specifies whether we're in the 'true' branch or the 'false' branch
  vector<pair<EvalObject *, bool>> conditions;

  // Traverse through the EvalObject tree for a variable value, returning a
  // reference to the EvalObject first not matching the current condition stack
  // Also returns the index where the match stopped
  pair<EvalObject *&, int> FindFirstNotMatchingConds(EvalObject *&value,
                                                     int index = 0);
};

// This is used for evaluating compile-time constants
class ConstantParser : public SingleCycleEvaluator {
public:
  ConstantParser(Parser::GlobalScope *_gs);
  BitConstant ParseConstexpr(Parser::Expression *expr);
};

class eval_error : public runtime_error {
  using runtime_error::runtime_error;
};
}
