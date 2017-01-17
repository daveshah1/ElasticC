#include "ParserStructures.hpp"
#include "ParserStatements.hpp"
#include "RCCParser.hpp"
#include "TemplateParser.hpp"
#include <algorithm>
using namespace std;

namespace RapidHLS {
namespace Parser {
Variable *Context::FindVariable(string name) {
  // First look in the current context
  // Searching in reverse ensures to pick up the most recent variable
  vector<Variable *> localVars = GetDeclaredVariables();
  auto foundLocalVar = find_if(localVars.rbegin(), localVars.rend(),
                               [&](Variable *v) { return v->name == name; });
  if (foundLocalVar == localVars.rend()) {
    if (parentContext == nullptr) {
      throw parse_error("variable ===" + name + "=== not found in any scope");
    } else {
      // Traverse up the tree of contexts
      return parentContext->FindVariable(name);
    }
  } else {
    return *foundLocalVar;
  }
};

bool Context::DoesVariableExist(string name) {
  try {
    FindVariable(name);
    return true;
  } catch (parse_error) {
    return false;
  }
}

GlobalScope *Context::GetGlobalScope() {
  if (parentContext == nullptr) {
    GlobalScope *gs = dynamic_cast<GlobalScope *>(this);
    if (gs == nullptr) {
      throw runtime_error("failed to reach global scope");
    } else {
      return gs;
    }
  } else {
    return parentContext->GetGlobalScope();
  }
};

vector<Templates::TemplateParameter *> Context::GetDefinedTemplateParameters() {
  return vector<Templates::TemplateParameter *>{};
};

pair<Context *, int> Context::FindTemplateParameter(string name) {
  vector<Templates::TemplateParameter *> localParams =
      GetDefinedTemplateParameters();
  auto localParam = find_if(
      localParams.begin(), localParams.end(),
      [name](Templates::TemplateParameter *p) { return p->name == name; });
  if (localParam == localParams.end()) {
    if (parentContext != nullptr) {
      return parentContext->FindTemplateParameter(name);
    } else {
      throw parse_error("===" + name + "=== is not a template parameter");
    }
  } else {
    return pair<Context *, int>(this,
                                distance(localParams.begin(), localParam));
  }
}

bool Context::IsTemplateParameter(string name) {
  try {
    FindTemplateParameter(name);
    return true;
  } catch (parse_error &e) {
    return false;
  }
}

vector<Variable *> Block::GetDeclaredVariables() {
  vector<Variable *> result;
  for (auto statement : content) {
    vector<Variable *> stVars = statement->GetVariableDeclarations();
    result.insert(result.end(), stVars.begin(), stVars.end());
  }
  return result;
}

ForLoop::ForLoop(Statement *_init, Expression *_cond, Expression *_inc,
                 Statement *_body, const AttributeSet &attr)
    : Statement(attr), initStatement(_init), condition(_cond),
      incrementer(_inc), body(_body){};

vector<Variable *> ForLoop::GetDeclaredVariables() {
  // Only interested in variables declared in the initialiser
  return initStatement->GetVariableDeclarations();
}

WhileLoop::WhileLoop(Expression *_cond, Statement *_body,
                     const AttributeSet &attr)
    : Statement(attr), condition(_cond), body(_body){};

IfStatement::IfStatement(Expression *_cond, Statement *_true, Statement *_false,
                         const AttributeSet &attr)
    : Statement(attr), condition(_cond), statementTrue(_true),
      statementFalse(_false){};

vector<Templates::TemplateParameter *>
UserStructure::GetDefinedTemplateParameters() {
  return params;
}

vector<Variable *> UserStructure::GetDeclaredVariables() {
  return vector<Variable *>{};
}

vector<Variable *> Function::GetDeclaredVariables() {
  // Only interested in variables declared in the function header, not its
  // content (which is the next context 'down')
  vector<Variable *> argvars;
  transform(arguments.begin(), arguments.end(), back_inserter(argvars),
            [](pair<Variable *, bool> a) { return a.first; });
  return argvars;
}

vector<Templates::TemplateParameter *>
Function::GetDefinedTemplateParameters() {
  return params;
};

vector<Variable *> HardwareBlock::GetDeclaredVariables() {
  vector<Variable *> result;
  result.insert(result.end(), inputs.begin(), inputs.end());
  result.insert(result.end(), outputs.begin(), outputs.end());
  return result;
}

vector<Variable *> GlobalScope::GetDeclaredVariables() {
  vector<Variable *> result;
  for (auto statement : statements) {
    vector<Variable *> stVars = statement->GetVariableDeclarations();
    result.insert(result.end(), stVars.begin(), stVars.end());
  }
  return result;
}
}
}
