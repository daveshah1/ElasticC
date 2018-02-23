#include "ParserStatements.hpp"
#include "ParserStructures.hpp"
#include <algorithm>
using namespace std;

namespace ElasticC {
namespace Parser {
vector<Variable *> Statement::GetVariableDeclarations() {
  // default is no variables declared by statement
  return vector<Variable *>();
}

Statement::Statement(){};
Statement::Statement(const AttributeSet &attr) : attributes(attr){};

BasicOperation::BasicOperation(OperationType _type,
                               const vector<Expression *> &_operands)
    : operType(_type), operands(_operands){};

Literal::Literal(BitConstant _val) : value(_val){};

VariableToken::VariableToken(Variable *_var) : var(_var){};

ArraySubscript::ArraySubscript(Expression *_base,
                               const vector<Expression *> &_index)
    : base(_base), index(_index){};

MemberAccess::MemberAccess(Expression *_base, string _mem)
    : base(_base), memberName(_mem){};

FunctionCall::FunctionCall(Function *_func,
                           const vector<Expression *> &_operands)
    : func(_func), operands(_operands){};

InitialiserList::InitialiserList(const vector<Expression *> &_values)
    : values(_values){};

const map<string, BuiltinType> BuiltinTokens = {
    {"sizeof", BuiltinType::SIZEOF},
    {"__widthof", BuiltinType::WIDTHOF},
    {"__length", BuiltinType::LENGTH},
    {"__min", BuiltinType::MIN},
    {"__max", BuiltinType::MAX}};

Builtin::Builtin(BuiltinType _type, Expression *_operand)
    : type(_type), operand(_operand){};

VariableDeclaration::VariableDeclaration(const vector<Variable *> &_declVar,
                                         const AttributeSet &attr)
    : Statement(attr), declaredVariables(_declVar) {
  for_each(declaredVariables.begin(), declaredVariables.end(),
           [=](Variable *v) { v->attributes = attributes; });
};

vector<Variable *> VariableDeclaration::GetVariableDeclarations() {
  return declaredVariables;
}

ReturnStatement::ReturnStatement(Expression *_retval) : returnValue(_retval){};

TemplateParamToken::TemplateParamToken(Context *_context, int _index)
    : pcontext(_context), index(_index){};

NullStatement_class NullStamement_obj;
NullStatement_class *NullStatement = &NullStamement_obj;

NullExpression_class NullExpression_obj;
NullExpression_class *NullExpression = &NullExpression_obj;
}
}
