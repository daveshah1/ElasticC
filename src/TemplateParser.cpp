#include "TemplateParser.hpp"
#include "EvalObject.hpp"
#include "Evaluator.hpp"
#include <algorithm>
using namespace std;

namespace RapidHLS {
namespace Parser {
namespace Templates {

BitConstantParameter::BitConstantParameter(string _name)
    : TemplateParameter(_name) {
  type = new AutoTypeSpecifier();
};

BitConstantParameter::BitConstantParameter(string _name,
                                           DataTypeSpecifier *_type)
    : TemplateParameter(_name), type(_type){};

void BitConstantParameter::Parse(RCCParser *parser, Context *ctx) {
  expr = parser->ParseExpression(vector<char>{',', '>'}, ctx);
}

BitConstant BitConstantParameter::GetValue(Evaluator *eval) {
  if (!wasSpecified)
    throw runtime_error("template parameter ===" + name + "=== not specified");
  TemplateParamContext tempTpContext;
  EvalObject *baseVal = eval->EvaluateExpression(expr);
  IntegerType *castType =
      dynamic_cast<IntegerType *>(type->Resolve(eval, &tempTpContext, baseVal));
  if (castType == nullptr)
    throw eval_error("template parameter ===" + name +
                     "=== must have scalar type");
  baseVal = new EvalCast(castType, baseVal);
  return baseVal->GetScalarConstValue(eval);
}

TemplateParameter *BitConstantParameter::Clone() const {
  return new BitConstantParameter(name, type);
}

int IntParameter::GetValue(Evaluator *eval) {
  return BitConstantParameter::GetValue(eval).intval();
};

TemplateParameter *IntParameter::Clone() const {
  return new IntParameter(name, type);
}

StringParameter::StringParameter(string _name) : TemplateParameter(_name){};

void StringParameter::Parse(RCCParser *parser, Context *ctx) {
  parser->code.Skip();
  value = parser->code.GetNextIdentOrLiteral(true);
}

string StringParameter::GetValue() { return value; };

TemplateParameter *StringParameter::Clone() const {
  return new StringParameter(name);
}

SelectorParameter::SelectorParameter(string _name,
                                     const vector<string> &_allowedValues)
    : TemplateParameter(_name), allowedValues(_allowedValues) {}

void SelectorParameter::Parse(RCCParser *parser, Context *ctx) {
  parser->code.Skip();
  string stringval = parser->code.GetNextIdentOrLiteral(true);
  auto iter = find(allowedValues.begin(), allowedValues.end(), stringval);
  if (iter == allowedValues.end()) {
    if (stringval == "") {
      throw parse_error(string("unexpected character ") +
                        parser->code.GetNext());
    } else {
      throw parse_error(stringval + " is not an allowed parameter value");
    }
  } else {
    index = distance(allowedValues.begin(), iter);
  }
};

int SelectorParameter::GetIndex() { return index; }

string SelectorParameter::GetValue() { return allowedValues[index]; }

TemplateParameter *SelectorParameter::Clone() const {
  return new SelectorParameter(name, allowedValues);
}

void DataTypeParameter::Parse(RCCParser *parser, Context *ctx) {
  value = parser->ParseDataType(ctx);
}

DataTypeParameter::DataTypeParameter(string _name) : TemplateParameter(_name){};

DataTypeSpecifier *DataTypeParameter::GetValue() { return value; }

TemplateParameter *DataTypeParameter::Clone() const {
  return new DataTypeParameter(name);
}

TemplateParser::TemplateParser(initializer_list<TemplateParameter *> _params)
    : params(_params){};

TemplateParser::TemplateParser(vector<TemplateParameter *> _params)
    : params(_params){};

void TemplateParser::Parse(RCCParser *parser, Context *ctx) {
  parser->code.Skip();

  if (!parser->code.CheckMatchAndGet('<')) {
    return;
  }

  for (int i = 0; i < params.size(); i++) {
    params[i]->wasSpecified = true;
    params[i]->Parse(parser, ctx);
    parser->code.Skip();
    if (parser->code.CheckMatchAndGet('>')) {
      break;
    } else if (!parser->code.CheckMatchAndGet(',')) {
      throw parse_error(
          string("expected parameter set separator (expected ',', got '") +
          parser->code.PeekNext() + string("')"));
    }
  }
};

vector<TemplateParameter *>
CloneParameterSet(const vector<TemplateParameter *> &params) {
  vector<TemplateParameter *> cloned;
  transform(params.begin(), params.end(), back_inserter(cloned),
            [](const TemplateParameter *p) { return p->Clone(); });
  return cloned;
};
}
}
}
