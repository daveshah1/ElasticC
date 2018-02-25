#include "Evaluator.hpp"
#include "EvalObject.hpp"
#include "EvaluatorState.hpp"
#include "TemplateParser.hpp"
#include "Util.hpp"
#include <algorithm>
#include <iterator>
#include <typeinfo>
using namespace std;
namespace ElasticC {
Evaluator::Evaluator(Parser::GlobalScope *_gs) : gs(_gs) {
  tpContext = new TemplateParamContext();
  tpContext->pContext = gs;
};

void Evaluator::AddVariable(EvaluatorVariable *var) {
  allVariables.push_back(var);
  vector<EvaluatorVariable *> children = var->GetAllChildren();
  for_each(children.begin(), children.end(),
           [this](EvaluatorVariable *child) { this->AddVariable(child); });
}

void Evaluator::AddVariable(EvaluatorVariable *var, Parser::Variable *orig) {
  AddVariable(var);
  parserVariables[orig] = var;
}

EvaluatorVariable *Evaluator::AddVariable(Parser::Variable *orig,
                                          bool is_block_input,
                                          bool is_block_output) {
  bool is_const =
      (find(orig->qualifiers.begin(), orig->qualifiers.end(),
            Parser::VariableQualifier::CONST) != orig->qualifiers.end());
  bool is_static =
      (find(orig->qualifiers.begin(), orig->qualifiers.end(),
            Parser::VariableQualifier::STATIC) != orig->qualifiers.end());
  // TODO: better initialiser list handling
  EvalObject *init = EvaluateExpression(orig->initialiser);
  DataType *type = orig->type->Resolve(this, tpContext, init);
  // TODO: unique naming in a better way
  string uname = (is_block_input || is_block_output)
                     ? orig->name
                     : (orig->name + "_ecc_" + to_string(GetUniqueID()));

  VariableDir dir{is_block_input, is_block_output,
                  is_block_input || is_block_output};
  EvaluatorVariable *var =
      EvaluatorVariable::Create(dir, uname, type, is_static);
  AddVariable(var, orig);
  if (is_const) {
    if (init == EvalNull) {
      throw eval_error("const variable ===" + orig->name +
                       "=== must have initialiser");
    } else if (!init->HasConstantValue(this)) {
      throw eval_error("initialiser for const variable ===" + orig->name +
                       "=== is not const itself");
    } else {
      init = init->GetConstantValue(this);
    }
  }
  if (init != EvalNull) {
    SetVariableValue(var, init);
  }
  return var;
}

vector<EvaluatorVariable *> Evaluator::GetAllVariables() {
  return allVariables;
}

EvaluatorVariable *Evaluator::GetVariableByParserVar(Parser::Variable *orig) {
  return parserVariables.at(orig);
};

EvalObject *Evaluator::ProcessFunctionCall(
    Parser::Function *func, vector<EvalObject *> arguments,
    vector<Parser::Templates::TemplateParameter *> templateParams) {

  CallStackEntry *cse = new CallStackEntry();
  cse->calledFunction = func;
  cse->savedParserVariables = parserVariables;
  cse->templateParams = templateParams;
  cse->oldTpContext = tpContext;

  tpContext = new TemplateParamContext();
  tpContext->pContext = func;
  tpContext->templateParams = templateParams;
  tpContext->parent = cse->oldTpContext;

  if (func->is_void) {
    cse->returnValue = nullptr;
  } else {
    // TODO: special case for auto return type?
    // would probably follow legacy ElasticC semantics rather than standard C++
    // semantics
    cse->returnValue = EvaluatorVariable::Create(
        VariableDir{false, false, false}, "retval_" + to_string(GetUniqueID()),
        func->returnType->Resolve(this, tpContext), false);
    AddVariable(cse->returnValue);
  }

  for (int i = 0; i < func->arguments.size(); i++) {
    if (i >= arguments.size()) {
      throw eval_error("too few arguments passed to function ===" + func->name +
                       "=== (expected " + to_string(func->arguments.size()) +
                       ", got " + to_string(arguments.size()) + ")");
    }
    SetVariableValue(AddVariable(func->arguments[i].first), arguments[i]);
  }

  callStack.push(cse);
  EvaluateStatement(func->body);
  callStack.pop();
  // update byref arguments
  for (int i = 0; i < func->arguments.size(); i++) {
    if (func->arguments[i].second) {
      arguments[i]->AssignValue(
          this, GetVariableValue(parserVariables[func->arguments[i].first]));
    }
  };
  parserVariables = cse->savedParserVariables;
  tpContext = cse->oldTpContext;



  if (func->is_void) {
    return EvalNull;
  } else {
    return GetVariableValue(cse->returnValue);
  }

  // TODO: delete cse?
};

BitConstant TemplateParamContext::GetNumericParameter(Evaluator *eval,
                                                      Parser::Context *origCtx,
                                                      int index) {
  if (origCtx == pContext) {
    Parser::Templates::BitConstantParameter *bcp =
        dynamic_cast<Parser::Templates::BitConstantParameter *>(
            templateParams.at(index));
    if (bcp == nullptr) {
      throw eval_error("parameter ===" + templateParams.at(index)->name +
                       "=== is not numeric");
    } else {
      return bcp->GetValue(eval);
    }
  } else {
    if (parent != nullptr) {
      return parent->GetNumericParameter(eval, origCtx, index);
    } else {
      throw eval_error("unable to resolve template parameter");
    }
  }
}

Parser::DataTypeSpecifier *
TemplateParamContext::GetTypeParameter(Evaluator *eval,
                                       Parser::Context *origCtx, int index) {

  if (origCtx == pContext) {
    Parser::Templates::DataTypeParameter *dtp =
        dynamic_cast<Parser::Templates::DataTypeParameter *>(
            templateParams.at(index));
    if (dtp == nullptr) {
      throw eval_error("parameter ===" + templateParams.at(index)->name +
                       "=== is not a type");
    } else {
      return dtp->GetValue();
    }
  } else {
    if (parent != nullptr) {
      return parent->GetTypeParameter(eval, origCtx, index);
    } else {
      throw eval_error("unable to resolve template parameter");
    }
  }
}

Evaluator::~Evaluator() {}

SingleCycleEvaluator::SingleCycleEvaluator(Parser::GlobalScope *_gs)
    : Evaluator(_gs){};

void SingleCycleEvaluator::AddVariable(EvaluatorVariable *var) {
  Evaluator::AddVariable(var);
  if (var->GetDir().is_input) {
    currentVariableValues[var] = new EvalVariable(var);
  } else {
    currentVariableValues[var] = new EvalDontCare(var->GetType());
  }
}

void SingleCycleEvaluator::SetVariableValue(EvaluatorVariable *var,
                                            EvalObject *value) {
  pair<EvalObject *&, int> toInsertCond =
      FindFirstNotMatchingConds(currentVariableValues[var], 0);
  IntegerType *intt = dynamic_cast<IntegerType *>(var->GetType());
  EvalObject *castValue;
  if (var->GetType()->Equals(value->GetDataType(this))) {
    castValue = value;
  } else {
    if (intt == nullptr) {
      // non-integer types require an exact match
      throw eval_error(
          "cannot convert type ===" + value->GetDataType(this)->GetName() +
          "=== to ===" + var->GetType()->GetName());
    } else {
      castValue = new EvalCast(intt, value);
    }
  }

  EvalObject *conditionalValue = castValue;
  // process conditions
  for (int i = conditions.size() - 1; i >= toInsertCond.second; i--) {
    if (conditions[i].second) {
      conditionalValue = new EvalSpecialOperation(
          SpecialOperationType::T_COND,
          vector<EvalObject *>{conditions[i].first, conditionalValue,
                               new EvalDontCare(var->GetType())});

    } else {
      conditionalValue = new EvalSpecialOperation(
          SpecialOperationType::T_COND,
          vector<EvalObject *>{conditions[i].first, conditionalValue,
                               new EvalDontCare(var->GetType())});
    }
  }
  toInsertCond.first = conditionalValue;
}

void SingleCycleEvaluator::EvaluateStatement(Parser::Statement *stmt) {
  try {

    Parser::VariableDeclaration *vardec;
    Parser::Block *blk;
    Parser::Expression *expr;
    Parser::IfStatement *ifst;
    Parser::ForLoop *forl;
    Parser::ReturnStatement *retst;

    if ((vardec = dynamic_cast<Parser::VariableDeclaration *>(stmt)) !=
        nullptr) {
      // Variable declaration
      for (auto var : vardec->declaredVariables) {
        Evaluator::AddVariable(var);
      }
    } else if ((blk = dynamic_cast<Parser::Block *>(stmt)) != nullptr) {
      // Block
      // TODO: handle the [[no_unroll]] attribute ?
      for (auto cstmt : blk->content) {
        EvaluateStatement(cstmt);
      }
    } else if ((expr = dynamic_cast<Parser::Expression *>(stmt))) {
      EvaluateExpression(expr);
    } else if ((ifst = dynamic_cast<Parser::IfStatement *>(stmt))) {
      conditions.push_back(
          pair<EvalObject *, bool>(EvaluateExpression(ifst->condition), true));
      EvaluateStatement(ifst->statementTrue);
      conditions.back().second = false;
      EvaluateStatement(ifst->statementFalse);
      conditions.pop_back();
    } else if ((forl = dynamic_cast<Parser::ForLoop *>(stmt))) {
      // TODO: handle the [[no_unroll]] attribute
      EvaluateStatement(forl->initStatement);
      bool loopDone = false;
      while (!loopDone) {
        EvalObject *cond = EvaluateExpression(forl->condition);
        if (!cond->HasConstantValue(this)) {
          throw eval_error(
              "for loop must have have compile-time constant condition");
        }
        if (cond->GetScalarConstValue(this).intval() == 0) {
          loopDone = true;
        } else {
          EvaluateStatement(forl->body);
          EvaluateStatement(forl->incrementer);
        }

      }
    } else if ((retst = dynamic_cast<Parser::ReturnStatement *>(stmt))) {
      // TODO: multiple `return` statements
      EvalObject *retVal = EvaluateExpression(retst->returnValue);
      SetVariableValue(callStack.top()->returnValue, retVal);
    } else if (stmt == Parser::NullStatement) {
      // do nothing
    } else {
      DEBUG_BREAKPOINT();
      throw eval_error("unsupported construct reached by evaluator");
    }
  } catch (eval_error &e) {
    PrintMessage(MSG_ERROR, e.what(), stmt->codeLine);
  }
}

void SingleCycleEvaluator::EvaluateBlock(Parser::HardwareBlock *block) {
  for (auto inp : block->inputs)
    Evaluator::AddVariable(inp, true, false);
  for (auto op : block->outputs)
    Evaluator::AddVariable(op, false, true);
  EvaluateStatement(block->body);
}

EvalObject *SingleCycleEvaluator::EvaluateExpression(Parser::Expression *expr) {
  Parser::BasicOperation *bop;
  Parser::Literal *lit;
  Parser::VariableToken *vart;
  Parser::ArraySubscript *arrs;
  Parser::MemberAccess *mema;
  Parser::FunctionCall *funcc;
  Parser::InitialiserList *il;
  Parser::Builtin *bt;
  Parser::TemplateParamToken *tpt;

  if ((bop = dynamic_cast<Parser::BasicOperation *>(expr)) != nullptr) {
    vector<EvalObject *> evalOperands;
    transform(
        bop->operands.begin(), bop->operands.end(), back_inserter(evalOperands),
        [this](Parser::Expression *exp) { return EvaluateExpression(exp); });
    return (new EvalBasicOperation(bop->operType, evalOperands))
        ->ApplyToState(this)
        ->GetValue(this);
  } else if ((lit = dynamic_cast<Parser::Literal *>(expr)) != nullptr) {
    return new EvalConstant(lit->value);
  } else if ((vart = dynamic_cast<Parser::VariableToken *>(expr)) != nullptr) {
    if (parserVariables.find(vart->var) != parserVariables.end()) {
      return new EvalVariable(parserVariables.at(vart->var));
    } else {
      vector<Parser::Variable *> globalVars = gs->GetDeclaredVariables();
      if(find(globalVars.begin(), globalVars.end(), vart->var) == globalVars.end())
        throw eval_error("variable " + vart->var->name +
                         " not declared properly");
      ConstantParser cp(gs);
      // TODO: const arrays
      BitConstant cnstVal = cp.ParseConstexpr(vart->var->initialiser);
      return new EvalConstant(cnstVal);
    }
  } else if ((arrs = dynamic_cast<Parser::ArraySubscript *>(expr)) != nullptr) {
    vector<EvalObject *> evalIndex;
    transform(
        arrs->index.begin(), arrs->index.end(), back_inserter(evalIndex),
        [this](Parser::Expression *ei) { return EvaluateExpression(ei); });
    return new EvalArrayAccess(EvaluateExpression(arrs->base), evalIndex);
  } else if ((mema = dynamic_cast<Parser::MemberAccess *>(expr)) != nullptr) {
    return new EvalStructAccess(EvaluateExpression(mema->base),
                                mema->memberName);
  } else if ((funcc = dynamic_cast<Parser::FunctionCall *>(expr)) != nullptr) {
    vector<EvalObject *> evalOperands;
    transform(
        funcc->operands.begin(), funcc->operands.end(), back_inserter(evalOperands),
        [this](Parser::Expression *exp) { return EvaluateExpression(exp); });
    return ProcessFunctionCall(
        funcc->func, evalOperands,
        funcc->params);
  } else if ((il = dynamic_cast<Parser::InitialiserList *>(expr)) != nullptr) {
    throw eval_error("initialiser list not permitted here");

  } else if ((bt = dynamic_cast<Parser::Builtin *>(expr)) != nullptr) {
    DataType *operandType = EvaluateExpression(bt->operand)->GetDataType(this);
    BitConstant value(0);
    if (bt->type == Parser::BuiltinType::SIZEOF) {
      value = BitConstant((operandType->GetWidth() + 7) / 8);
    } else if (bt->type == Parser::BuiltinType::WIDTHOF) {
      value = BitConstant(operandType->GetWidth());
    } else if (bt->type == Parser::BuiltinType::LENGTH) {
      // TODO: `stream2d`s with two dimensions
      vector<int> dim = operandType->GetDimensions();
      if (dim.size() < 1) {
        throw eval_error("cannot call __length on scalar");
      }
      value = BitConstant(dim[0]);
    } else if (bt->type == Parser::BuiltinType::MIN) {
      IntegerType *it = dynamic_cast<IntegerType *>(operandType);
      if (it == nullptr) {
        throw eval_error("cannot call __min on non-integer");
      }
      if (it->is_signed) { // signed min is 0b1000...
        value.bits.resize(it->width);
        value.bits.back() = true;
        value.is_signed = true;
      } else { // unsigned min is 0
        value = BitConstant(0);
        value.is_signed = false;
      }
    } else if (bt->type == Parser::BuiltinType::MAX) {
      IntegerType *it = dynamic_cast<IntegerType *>(operandType);
      if (it == nullptr) {
        throw eval_error("cannot call __max on non-integer");
      }
      if (it->is_signed) { // signed max is 0b0111...
        value.bits.resize(it->width, true);
        value.bits.back() = false;
        value.is_signed = true;
      } else { // unsigned max is 0b1111...
        value.bits.resize(it->width, true);
        value.is_signed = false;
      }
    }
    return new EvalConstant(value);
  } else if ((tpt = dynamic_cast<Parser::TemplateParamToken *>(expr)) !=
             nullptr) {
    return new EvalConstant(
        tpContext->GetNumericParameter(this, tpt->pcontext, tpt->index));
  } else if (expr == Parser::NullExpression) {
    return EvalNull;
  } else {
    throw runtime_error(string("FIXME: unsupported expression token type ") +
                        string(typeid(*expr).name()));
  }
};

pair<EvalObject *&, int>
SingleCycleEvaluator::FindFirstNotMatchingConds(EvalObject *&value, int index) {
  if (index >= conditions.size())
    return pair<EvalObject *&, int>(value, index); // end of the condition stack
  EvalSpecialOperation *eso = dynamic_cast<EvalSpecialOperation *>(value);
  if ((eso == nullptr) || (eso->type != SpecialOperationType::T_COND))
    return pair<EvalObject *&, int>(value, index); // not a conditional at all
  if (eso->GetOperands()[0] != conditions[index].first)
    return pair<EvalObject *&, int>(value, index); // conditions do not match

  // is a match, just follow the right branch
  if (conditions[index].second)
    return FindFirstNotMatchingConds(eso->GetOperands()[1], index + 1);
  else
    return FindFirstNotMatchingConds(eso->GetOperands()[2], index + 2);
}

EvalObject *SingleCycleEvaluator::GetVariableValue(EvaluatorVariable *var) {
  return currentVariableValues.at(var);
}

EvaluatedBlock SingleCycleEvaluator::GetEvaluatedBlock() {
  return EvaluatedBlock{currentVariableValues, parserVariables, this};
}

SingleCycleEvaluator::~SingleCycleEvaluator() {}

ConstantParser::ConstantParser(Parser::GlobalScope *_gs)
    : SingleCycleEvaluator(gs){};

BitConstant ConstantParser::ParseConstexpr(Parser::Expression *expr) {
  return EvaluateExpression(expr)->GetConstantValue(this)->GetScalarConstValue(
      this);
}
} // namespace ElasticC
