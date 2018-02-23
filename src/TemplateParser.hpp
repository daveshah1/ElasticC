#pragma once
#include "DataTypes.hpp"
#include "ECCParser.hpp"
#include <initializer_list>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
using namespace std;
namespace ElasticC {
class Evaluator;
namespace Parser {
class DataTypeSpecifier;
namespace Templates {
class TemplateParameter {
public:
  TemplateParameter(string _name);
  // Parse the parameter using the current parser
  virtual void Parse(ECCParser *parser, Context *ctx) = 0;
  // Create a clone of the parameter; however ignoring its current value
  virtual TemplateParameter *Clone() const = 0;

  string name;
  bool wasSpecified = false;
};

class BitConstantParameter : public TemplateParameter {
public:
  BitConstantParameter(string _name);
  BitConstantParameter(string _name, DataTypeSpecifier *_type);
  void Parse(ECCParser *parser, Context *ctx);
  BitConstant GetValue(Evaluator *eval);
  virtual TemplateParameter *Clone() const;

protected:
  DataTypeSpecifier *type;
  Expression *expr = NullExpression;
};

class IntParameter : public BitConstantParameter {
public:
  using BitConstantParameter::BitConstantParameter;
  int GetValue(Evaluator *eval);
  TemplateParameter *Clone() const;
};

class StringParameter : public TemplateParameter {
public:
  StringParameter(string _name);
  void Parse(ECCParser *parser, Context *ctx);
  string GetValue();
  TemplateParameter *Clone() const;

private:
  string value;
};

class SelectorParameter : public TemplateParameter {
public:
  SelectorParameter(string _name, const vector<string> &_allowedValues);
  void Parse(ECCParser *parser, Context *ctx);
  string GetValue();
  int GetIndex();
  TemplateParameter *Clone() const;

private:
  vector<string> allowedValues;
  int index = 0;
};

class DataTypeParameter : public TemplateParameter {
public:
  DataTypeParameter(string _name);
  void Parse(ECCParser *parser, Context *ctx);
  DataTypeSpecifier *GetValue();
  TemplateParameter *Clone() const;

private:
  DataTypeSpecifier *value;
};

class TemplateParser {
public:
  TemplateParser(vector<TemplateParameter *> _params);
  TemplateParser(initializer_list<TemplateParameter *> _params);

  void Parse(ECCParser *parser, Context *ctx);

private:
  vector<TemplateParameter *> params;
};

vector<TemplateParameter *>
CloneParameterSet(const vector<TemplateParameter *> &params);
}
}
}
