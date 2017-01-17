#include "DataTypeSpecifier.hpp"
#include "Evaluator.hpp"
#include "ParserStatements.hpp"
#include "ParserStructures.hpp"
#include "TemplateParser.hpp"
#include <algorithm>
#include <iterator>
using namespace std;

namespace RapidHLS {
namespace Parser {
vector<Templates::TemplateParameter *> &DataTypeSpecifier::Params() {
  return _params;
}

const map<string, BasicDataType> basicTypeNames = {
    {"unsigned", BasicDataType::UNSIGNED},
    {"signed", BasicDataType::SIGNED},
    {"stream", BasicDataType::STREAM},
    {"stream2d", BasicDataType::STREAM2D},
    {"ram", BasicDataType::RAM},
    {"rom", BasicDataType::ROM}};

BasicDataTypeSpecifier::BasicDataTypeSpecifier(BasicDataType _type)
    : type(_type) {
  // Initialise the set of template parameters for the basic types
  switch (type) {
  case BasicDataType::UNSIGNED:
  case BasicDataType::SIGNED:
    params = vector<Templates::TemplateParameter *>{
        new Templates::IntParameter("width")};
    break;
  case BasicDataType::STREAM:
    params = vector<Templates::TemplateParameter *>{
        new Templates::DataTypeParameter("baseType"),
        new Templates::IntParameter("length")};
    break;
  case BasicDataType::STREAM2D:
    params = vector<Templates::TemplateParameter *>{
        new Templates::DataTypeParameter("baseType"),
        new Templates::IntParameter("length"),
        new Templates::IntParameter("width"),
        new Templates::IntParameter("lineWidth")};
    break;
  case BasicDataType::RAM:
  case BasicDataType::ROM:
    params = vector<Templates::TemplateParameter *>{
        new Templates::DataTypeParameter("baseType"),
        new Templates::IntParameter("length")};
    break;
  }
}

BasicDataTypeSpecifier::BasicDataTypeSpecifier(BasicDataType _type,
                                               string strParams)
    : BasicDataTypeSpecifier(_type) {
  // The set of parameters has already been set up by the basic constructor;
  // just use a temporary parser to parse them from the string
  ParserState tempCode(strParams);
  GlobalScope tempScope;
  RCCParser tempParser(tempCode, tempScope);
  Templates::TemplateParser tempTParser(params);
  tempTParser.Parse(&tempParser, &tempScope);
};

DataType *BasicDataTypeSpecifier::Resolve(Evaluator *currentEval,
                                          TemplateParamContext *tpContext,
                                          EvalObject *value) {
  switch (type) {
  case BasicDataType::UNSIGNED:
    return new IntegerType(
        dynamic_cast<Templates::IntParameter *>(params[0])->GetValue(
            currentEval),
        false);
  case BasicDataType::SIGNED:
    return new IntegerType(
        dynamic_cast<Templates::IntParameter *>(params[0])->GetValue(
            currentEval),
        true);
  case BasicDataType::STREAM:
    return new StreamType(
        dynamic_cast<Templates::DataTypeParameter *>(params[0])
            ->GetValue()
            ->Resolve(currentEval, tpContext),
        false, dynamic_cast<Templates::IntParameter *>(params[1])->GetValue(
                   currentEval));
  case BasicDataType::STREAM2D:
    return new StreamType(
        dynamic_cast<Templates::DataTypeParameter *>(params[0])
            ->GetValue()
            ->Resolve(currentEval, tpContext),
        true, dynamic_cast<Templates::IntParameter *>(params[1])->GetValue(
                  currentEval),
        dynamic_cast<Templates::IntParameter *>(params[2])->GetValue(
            currentEval),
        dynamic_cast<Templates::IntParameter *>(params[3])->GetValue(
            currentEval));
  case BasicDataType::RAM:
  case BasicDataType::ROM:
    DataType *baseType = dynamic_cast<Templates::DataTypeParameter *>(params[0])
                             ->GetValue()
                             ->Resolve(currentEval, tpContext);
    if (dynamic_cast<IntegerType *>(baseType) == nullptr) {
      throw eval_error("base type of memory must be an integer");
    }
    RAMType *ramType = new RAMType(
        *dynamic_cast<IntegerType *>(baseType),
        dynamic_cast<Templates::IntParameter *>(params[1])->GetValue(
            currentEval));
    ramType->is_rom = (type == BasicDataType::ROM);
    return ramType;
  }
}

StructureTypeSpecifier::StructureTypeSpecifier(UserStructure *_ustruct)
    : ustruct(_ustruct) {
  params = Templates::CloneParameterSet(ustruct->params);
};

DataType *StructureTypeSpecifier::Resolve(Evaluator *currentEval,
                                          TemplateParamContext *tpContext,
                                          EvalObject *value) {
  // This binds the structure definition to an actual StructureType, applying
  // template parameters etc
  TemplateParamContext *structTPContext = new TemplateParamContext();
  structTPContext->parent = tpContext;
  structTPContext->pContext = ustruct;
  structTPContext->templateParams = params;
  vector<DataStructureItem> content;
  transform(ustruct->members.begin(), ustruct->members.end(),
            back_inserter(content), [&](Parser::Variable *member) {
              return DataStructureItem(
                  member->name,
                  member->type->Resolve(
                      currentEval, structTPContext,
                      currentEval->EvaluateExpression(member->initialiser)));
            });
  return new StructureType(ustruct->name, content);
};

vector<Templates::TemplateParameter *> &StructureTypeSpecifier::Params() {
  return params;
}

ArrayTypeSpecifier::ArrayTypeSpecifier(DataTypeSpecifier *_baseType,
                                       Expression *_length)
    : baseType(_baseType), length(_length){};

DataType *ArrayTypeSpecifier::Resolve(Evaluator *currentEval,
                                      TemplateParamContext *tpContext,
                                      EvalObject *value) {
  return new ArrayType(baseType->Resolve(currentEval, tpContext),
                       currentEval->EvaluateExpression(length)
                           ->GetScalarConstValue(currentEval)
                           .intval());
}

AutoTypeSpecifier::AutoTypeSpecifier(){};

DataType *AutoTypeSpecifier::Resolve(Evaluator *currentEval,
                                     TemplateParamContext *tpContext,
                                     EvalObject *value) {
  if (value == EvalNull) {
    throw eval_error("use of ===auto=== type requires a value to be specified");
  }
  return value->GetDataType(currentEval);
}

TemplateParamTypeSpecifier::TemplateParamTypeSpecifier(Context *_pcontext,
                                                       int _pindex)
    : pcontext(_pcontext), pindex(_pindex){};

DataType *TemplateParamTypeSpecifier::Resolve(Evaluator *currentEval,
                                              TemplateParamContext *tpContext,
                                              EvalObject *value) {
  return tpContext->GetTypeParameter(currentEval, pcontext, pindex)
      ->Resolve(currentEval, tpContext);
}

DecltypeSpecifier::DecltypeSpecifier(Expression *_operand)
    : operand(_operand) {}

DataType *DecltypeSpecifier::Resolve(Evaluator *currentEval,
                                     TemplateParamContext *tpContext,
                                     EvalObject *value) {
  return currentEval->EvaluateExpression(operand)->GetDataType(currentEval);
}
}
}
