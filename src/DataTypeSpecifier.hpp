#pragma once
#include "EvalObject.hpp"
#include <map>
#include <string>
#include <vector>
using namespace std;

namespace RapidHLS {
class DataType;
class Evaluator;
class EvalObject;
class TemplateParamContext;
namespace Parser {
class UserStructure;
class Expression;
class Context;
namespace Templates {
class TemplateParameter;
}
/*This represents a data type specifier at the time of parsing, resolved
 * during evaluation to an actual DataType */

class DataTypeSpecifier {
public:
  // Resolve the DataTypeSpecifier to an actual type; given the current
  // evaluator, and variable initialiser (only used for auto type)
  virtual DataType *Resolve(Evaluator *currentEval,
                            TemplateParamContext *tpContext,
                            EvalObject *value = EvalNull) = 0;
  // Return a reference to a vector of template parameters for the type
  virtual vector<Templates::TemplateParameter *> &Params();

private:
  vector<Templates::TemplateParameter *> _params;
};

enum class BasicDataType {
  UNSIGNED,
  SIGNED,
  STREAM,
  STREAM2D,
  RAM,
  ROM,
};

extern const map<string, BasicDataType> basicTypeNames;

// One of the fundamental built in named types
class BasicDataTypeSpecifier : public DataTypeSpecifier {
public:
  BasicDataTypeSpecifier(BasicDataType _type);
  // Special form where the parameters are fixed before runtime (i.e. builtin
  // typedefs); and thus sepecified as a string
  BasicDataTypeSpecifier(BasicDataType _type, string strParams);

  DataType *Resolve(Evaluator *currentEval, TemplateParamContext *tpContext,
                    EvalObject *value = EvalNull);
  vector<Templates::TemplateParameter *> &Params();

private:
  BasicDataType type;
  vector<Templates::TemplateParameter *> params;
};

// Specifies a user defined structure
class StructureTypeSpecifier : public DataTypeSpecifier {
public:
  StructureTypeSpecifier(UserStructure *_ustruct);
  DataType *Resolve(Evaluator *currentEval, TemplateParamContext *tpContext,
                    EvalObject *value = EvalNull);
  // This is included as hopefully we will have template structs at some point
  // e.g. for pixel types which have many possible widths and require
  // significant copy and pasting in the legacy libelastic
  vector<Templates::TemplateParameter *> &Params();

private:
  UserStructure *ustruct;
  vector<Templates::TemplateParameter *> params;
};

// Specifies an array of some other type
class ArrayTypeSpecifier : public DataTypeSpecifier {
public:
  ArrayTypeSpecifier(DataTypeSpecifier *_baseType, Expression *_length);
  DataType *Resolve(Evaluator *currentEval, TemplateParamContext *tpContext,
                    EvalObject *value = EvalNull);

private:
  DataTypeSpecifier *baseType;
  Expression *length;
};

// Specifies automatic typing, e.g. the type of the variable is defined as the
// type of the initialiser
class AutoTypeSpecifier : public DataTypeSpecifier {
public:
  AutoTypeSpecifier();
  DataType *Resolve(Evaluator *currentEval, TemplateParamContext *tpContext,
                    EvalObject *value = EvalNull);
};

// Specifies that the data type is specified as a template parameter
// It is specified as an index into the set of template parameters for the
// current object
class TemplateParamTypeSpecifier : public DataTypeSpecifier {
public:
  TemplateParamTypeSpecifier(Context *_pcontext, int _pindex);
  DataType *Resolve(Evaluator *currentEval, TemplateParamContext *tpContext,
                    EvalObject *value = EvalNull);

private:
  Context *pcontext;
  int pindex;
};

// C++ decltype specifier, which has the type of the result of an expression
class DecltypeSpecifier : public DataTypeSpecifier {
public:
  DecltypeSpecifier(Expression *_operand);
  DataType *Resolve(Evaluator *currentEval, TemplateParamContext *tpContext,
                    EvalObject *value = EvalNull);

private:
  Expression *operand;
};
}
}
