#pragma once
#include "Attributes.hpp"
#include "BitConstant.hpp"
#include "DataTypes.hpp"
#include "Operations.hpp"
#include "ParserCore.hpp"
#include "hdl/HDLDesign.hpp"
#include <map>
#include <string>
#include <vector>
namespace RapidHLS {
class Evaluator;
class EvalConstant;
class EvaluatorVariable;
// This is effectively anything which has a value (whether constant or an
// expression; scalar or array)
// It is generated after parsing
class EvalObject {
public:
  EvalObject();

  AttributeSet attributes;
  // Return a string identifier
  virtual string GetID();
  // Return the data type of the EvalObject, if it can be determined
  virtual DataType *GetDataType(Evaluator *state) = 0;
  // Return true if the EvalObject is entirely constant (this is faster than
  // GetConstantValue when the actual value is unimportant)
  virtual bool HasConstantValue(Evaluator *state);
  // Return the constant value if it has one, or throw if it doesn't
  // This should apply constant folding
  virtual EvalObject *GetConstantValue(Evaluator *state);
  virtual BitConstant GetScalarConstValue(Evaluator *state);

  // Apply array subscripting to the EvalObject, if possible (otherwise throw)
  virtual EvalObject *ApplyArraySubscriptRead(Evaluator *state,
                                              vector<EvalObject *> subscript);
  virtual void ApplyArraySubscriptWrite(Evaluator *state,
                                        vector<EvalObject *> subscript,
                                        EvalObject *value);

  // Attempt to access structure member, if possible (otherwise throw)
  virtual EvalObject *GetStructureMember(Evaluator *state, string name);
  virtual void AssignStructureMember(Evaluator *state, string name,
                                     EvalObject *value);

  // Apply 'side effects' of the EvalObject (e.g. assignment) to a variable
  // Returns the result of the application (if N/A returns itself)
  // This should also process array subscripting, structure access, etc
  virtual EvalObject *ApplyToState(Evaluator *state);
  // Apply the effect of an assignment to the EvalObject to the current state
  virtual void AssignValue(Evaluator *state, EvalObject *value);
  // Get the value of the EvalObject applying variable substitution, indexing,
  // etc
  virtual EvalObject *GetValue(Evaluator *state);
  // Return all operands
  virtual vector<EvalObject *> GetOperands();
  // Returns whether or not the underlying object can be pushed into; e.g. is it
  // a reference to a stream, FIFO, etc
  virtual bool CanPushInto();
  // Process a push into a stream
  virtual EvalObject *ApplyPushInto(Evaluator *state, EvalObject *value);
  // Synthesise the EvalObject into a HDL design, connecting the output to a
  // given signal. Throws if the EvalObject is not synthesisable
  virtual void Synthesise(HDLGen::HDLDesign *design,
                          HDLGen::HDLSignal *outputNet);

protected:
  int base_id = 0;
};

// Represents a reference to a variable
class EvalVariable : public EvalObject {
public:
  EvalVariable(EvaluatorVariable *_var);
  string GetID();
  DataType *GetDataType(Evaluator *state);
  bool HasConstantValue(Evaluator *state);
  EvalObject *GetConstantValue(Evaluator *state);
  EvalObject *ApplyArraySubscriptRead(Evaluator *state,
                                      vector<EvalObject *> subscript);
  void ApplyArraySubscriptWrite(Evaluator *state,
                                vector<EvalObject *> subscript,
                                EvalObject *value);
  EvalObject *GetStructureMember(Evaluator *state, string name);
  void AssignStructureMember(Evaluator *state, string name, EvalObject *value);
  void AssignValue(Evaluator *state, EvalObject *value);
  EvalObject *GetValue(Evaluator *state);

private:
  EvaluatorVariable *var;
};

// Represents a basic scalar constant
class EvalConstant : public EvalObject {
public:
  EvalConstant(BitConstant _val);
  // Return a string identifier
  string GetID();
  bool HasConstantValue(Evaluator *state);
  EvalObject *GetConstantValue(Evaluator *state);
  DataType *GetDataType(Evaluator *state);
  BitConstant GetScalarConstValue(Evaluator *state);

private:
  BitConstant val;
};

// Represents an array of arbitrary values (could be constants or expressions)
class EvalArray : public EvalObject {
public:
  EvalArray(ArrayType *_arrType, const vector<EvalObject *> &_items);
  string GetID();
  bool HasConstantValue(Evaluator *state);
  DataType *GetDataType(Evaluator *state);
  EvalObject *GetConstantValue(Evaluator *state);
  EvalObject *ApplyArraySubscriptRead(Evaluator *state,
                                      vector<EvalObject *> subscript);
  void ApplyArraySubscriptWrite(Evaluator *state,
                                vector<EvalObject *> subscript,
                                EvalObject *value);
  vector<EvalObject *> GetOperands();

private:
  ArrayType *arrType;
  vector<EvalObject *> items;
};

// Represents a struct containing arbitrary values
class EvalStruct : public EvalObject {
public:
  EvalStruct(StructureType *_structType,
             const map<string, EvalObject *> _items);
  string GetID();
  bool HasConstantValue(Evaluator *state);
  DataType *GetDataType(Evaluator *state);
  EvalObject *GetConstantValue(Evaluator *state);
  EvalObject *GetStructureMember(Evaluator *state, string name);
  void AssignStructureMember(Evaluator *state, string name, EvalObject *value);
  vector<EvalObject *> GetOperands();

private:
  StructureType *structType;
  map<string, EvalObject *> items;
};

// Represents an array access with non-constant index
class EvalArrayAccess : public EvalObject {
public:
  EvalArrayAccess(EvalObject *_base, vector<EvalObject *> _index);
  string GetID();
  DataType *GetDataType(Evaluator *state);
  bool HasConstantValue(Evaluator *state);
  EvalObject *GetConstantValue(Evaluator *state);
  EvalObject *ApplyArraySubscriptRead(Evaluator *state,
                                      vector<EvalObject *> subscript);
  void ApplyArraySubscriptWrite(Evaluator *state,
                                vector<EvalObject *> subscript,
                                EvalObject *value);
  EvalObject *GetStructureMember(Evaluator *state, string name);
  void AssignStructureMember(Evaluator *state, string name, EvalObject *value);

  EvalObject *ApplyToState(Evaluator *state);
  void AssignValue(Evaluator *state, EvalObject *value);
  vector<EvalObject *> GetOperands();
  EvalObject *GetValue(Evaluator *state);
  EvalObject *GetReference(Evaluator *state);

private:
  EvalObject *base;
  vector<EvalObject *> index;
};

// Represents structure memeber access
class EvalStructAccess : public EvalObject {
public:
  EvalStructAccess(EvalObject *_base, string _member);
  string GetID();
  DataType *GetDataType(Evaluator *state);
  bool HasConstantValue(Evaluator *state);
  EvalObject *GetConstantValue(Evaluator *state);
  EvalObject *ApplyArraySubscriptRead(Evaluator *state,
                                      vector<EvalObject *> subscript);
  void ApplyArraySubscriptWrite(Evaluator *state,
                                vector<EvalObject *> subscript,
                                EvalObject *value);
  EvalObject *GetStructureMember(Evaluator *state, string name);
  void AssignStructureMember(Evaluator *state, string name, EvalObject *value);

  EvalObject *ApplyToState(Evaluator *state);
  void AssignValue(Evaluator *state, EvalObject *value);
  vector<EvalObject *> GetOperands();
  EvalObject *GetValue(Evaluator *state);

private:
  EvalObject *base;
  string member;
};

// Represents a cast
class EvalCast : public EvalObject {
public:
  EvalCast(IntegerType *_castTo, EvalObject *_operand);
  string GetID();
  DataType *GetDataType(Evaluator *state);
  bool HasConstantValue(Evaluator *state);
  EvalObject *GetConstantValue(Evaluator *state);
  BitConstant GetScalarConstValue(Evaluator *state);
  vector<EvalObject *> GetOperands();
  EvalObject *GetValue(Evaluator *state);

private:
  IntegerType *castTo;
  EvalObject *operand;
};

// Represents a basic operation as used by the parser
// These may have side effects (e.g. operator+=) but these will be eliminated
// after calling ApplyToState
class EvalBasicOperation : public EvalObject {
public:
  EvalBasicOperation(OperationType _type, vector<EvalObject *> _operands);
  string GetID();
  DataType *GetDataType(Evaluator *state);
  bool HasConstantValue(Evaluator *state);
  EvalObject *GetConstantValue(Evaluator *state);
  EvalObject *ApplyToState(Evaluator *state);
  vector<EvalObject *> GetOperands();
  EvalObject *GetValue(Evaluator *state);

  // Returns true if the operation is a push or assignment and therefore has
  // non-numeric result (equal to operand index 1)
  bool NonNumericAllowed();

  // Return the result of the operation given some operands; ignoring side
  // effects
  EvalObject *GetResult(Evaluator *state, const vector<EvalObject *> &operands);
  EvalObject *GetResult(Evaluator *state, const vector<EvalObject *> &operands,
                        OperationType type);

private:
  OperationType type;
  vector<EvalObject *> operands;
};

// Represents a "special" operation not directly specified by the user by
// inferred from some other code
enum class SpecialOperationType {
  T_COND,    // the ternary conditional operator (generated from if statements)
  ARRAY_SEL, // array subscript access; operands : all array children followed
             // by index
  ARRAY_WRITE, // array item conditional write; operands : original value, new
               // value, write index
               // parameters : index of item
};

class EvalSpecialOperation : public EvalObject {
public:
  EvalSpecialOperation(SpecialOperationType _type,
                       vector<EvalObject *> _operands,
                       vector<BitConstant> _parameters = {});
  string GetID();
  DataType *GetDataType(Evaluator *state);
  bool HasConstantValue(Evaluator *state);
  EvalObject *GetConstantValue(Evaluator *state);
  EvalObject *ApplyToState(Evaluator *state);
  void AssignValue(Evaluator *state, EvalObject *value);
  vector<EvalObject *> GetOperands();
  EvalObject *GetValue(Evaluator *state);
  SpecialOperationType type;

private:
  vector<EvalObject *> operands;
  vector<BitConstant> parameters;
};

// Represents a pipeline register
class EvalRegister : public EvalObject {
public:
  EvalRegister(EvalObject *_input);
  string GetID();
  DataType *GetDataType(Evaluator *state);
  bool HasConstantValue(Evaluator *state);
  EvalObject *GetConstantValue(Evaluator *state);
  EvalObject *ApplyToState(Evaluator *state);
  vector<EvalObject *> GetOperands();
  EvalObject *GetValue(Evaluator *state);

private:
  EvalObject *input;
};

// Represents a 'don't care' value
class EvalDontCare : public EvalObject {
public:
  EvalDontCare(DataType *_type);
  string GetID();
  DataType *GetDataType(Evaluator *state);

  bool HasConstantValue(Evaluator *state);
  EvalObject *GetConstantValue(Evaluator *state);
  EvalObject *ApplyArraySubscriptRead(Evaluator *state,
                                      vector<EvalObject *> subscript);
  void ApplyArraySubscriptWrite(Evaluator *state,
                                vector<EvalObject *> subscript,
                                EvalObject *value);
  EvalObject *GetStructureMember(Evaluator *state, string name);
  void AssignStructureMember(Evaluator *state, string name, EvalObject *value);
  EvalObject *ApplyToState(Evaluator *state);
  void AssignValue(Evaluator *state, EvalObject *value);
  vector<EvalObject *> GetOperands();
  EvalObject *GetValue(Evaluator *state);

private:
  DataType *type;
};

// Represent a 'friendly null' - designed as a placeholder where no value exists
// but less likely to cause problems than a real nullptr
// Any method call will result in a throw
class EvalNull_class : public EvalObject {
public:
  EvalNull_class();
  string GetID();
  DataType *GetDataType(Evaluator *state);

  bool HasConstantValue(Evaluator *state);
  EvalObject *GetConstantValue(Evaluator *state);
  EvalObject *ApplyArraySubscriptRead(Evaluator *state,
                                      vector<EvalObject *> subscript);
  void ApplyArraySubscriptWrite(Evaluator *state,
                                vector<EvalObject *> subscript,
                                EvalObject *value);
  EvalObject *GetStructureMember(Evaluator *state, string name);
  void AssignStructureMember(Evaluator *state, string name, EvalObject *value);
  EvalObject *ApplyToState(Evaluator *state);
  void AssignValue(Evaluator *state, EvalObject *value);
  vector<EvalObject *> GetOperands();
  EvalObject *GetValue(Evaluator *state);

  const string nullMessage =
      "null in evaluation tree (probably an internal error, please report)";
};

extern EvalNull_class *EvalNull;
}
