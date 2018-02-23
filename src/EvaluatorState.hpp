#pragma once
#include "Attributes.hpp"
#include "DataTypes.hpp"
#include "Operations.hpp"
#include "ParserCore.hpp"
#include "ParserStructures.hpp"
#include "SynthContext.hpp"
#include <map>
#include <stack>
#include <string>
#include <vector>
namespace ElasticC {

class EvalObject;
class EvaluatorVariable;
class Evaluator;
/*
This keeps track of the state of the "Evaluator", which generates expressions
for all variables, array and memory accesses, etc; from the parsed code. From
this it can then perform pipelining if required and output VHDL or a Polymer
netlist.

It wraps all Variable objects created by the parser to create 'child' variables
used by arrays, structures and RAM

*/

struct MemoryDeviceParameters {
  bool canRead = true;
  bool canWrite = true;
  // Number of clock cycles from address out to data valid
  int readLatency = 1;
  // Whether or not a wren pin exists
  bool hasWren = true;
  // Whether or not a rden pin exists
  bool hasRden = false;
  // Whether or not it has separate read and write ports
  bool seperateRWports = false;
};

struct VariableDir {
  VariableDir(bool _input, bool _output, bool _toplvl);
  bool is_input = false; // Is an input from VHDL (memory data in, static var
  // value, or toplevel input) of some kind to ElasticC
  bool is_output = false; // Is an output to VHDL (write enable, written value,
  // or toplevel output) from ElasticC
  bool is_toplevel = false; // Is exposed in the final design entity's port
};

class EvaluatorVariable {
public:
  EvaluatorVariable(VariableDir _dir);
  EvaluatorVariable(VariableDir _dir, string _name);
  EvaluatorVariable(VariableDir _dir, string _name, const AttributeSet &_attr);
  static EvaluatorVariable *Create(VariableDir _dir, string _name,
                                   DataType *_type, bool _is_static);

  string name;
  AttributeSet attributes;
  virtual DataType *GetType() = 0;

  // Return any "child" variables in order
  // This applies to non-scalars only
  virtual vector<EvaluatorVariable *> GetArrayChildren();
  // Return all variables that need to be synthesised as a result of this one
  // (include structure children and special variables such as write enable)
  virtual vector<EvaluatorVariable *> GetAllChildren();
  // Return a named child - applies to structures, and possible RAM, only
  // at the moment
  virtual EvaluatorVariable *GetChildByName(string name);

  // Return true if it's a scalar, false otherwise
  virtual bool IsScalar() = 0;

  // Return true if it has a default value, false otherwise
  virtual bool HasDefaultValue();
  virtual BitConstant GetDefaultValue();

  // All these should be overriden if non-trivial behaviour is required
  virtual EvalObject *HandleRead(Evaluator *genst);
  // Handle a write - overriden for static variables
  virtual void HandleWrite(Evaluator *genst, EvalObject *value);
  // Return true if array subscripting is non-trivial
  virtual bool IsNonTrivialArrayAccess();
  // Handle array subscripted read given index (which may be multidimensional)
  virtual EvalObject *HandleSubscriptedRead(Evaluator *genst,
                                            vector<EvalObject *> index);
  // Handle array subscripted write given index (which may be multidimensional)
  virtual void HandleSubscriptedWrite(Evaluator *genst,
                                      vector<EvalObject *> index,
                                      EvalObject *value);

  // Handle a push (using operator<<) - stream/stream2d types and FIFOs
  virtual void HandlePush(Evaluator *genst, EvalObject *value);
  // Handle a pop (using operator>>) - possible FIFO/stack type
  virtual EvalObject *HandlePop(Evaluator *genst);

  // These are used for packing and unpacking structures and arrays
  virtual void SetBitOffset(int _bitoffset);
  virtual int GetBitOffset();
  virtual VariableDir GetDir();

  // Add the variable to the HDL design, generating any necessary supporting
  // logic
  virtual void Synthesise(SynthContext &sc);

protected:
  VariableDir dir;

private:
  int bitoffset = 0;
};

class ScalarEvaluatorVariable : public EvaluatorVariable {
public:
  ScalarEvaluatorVariable(VariableDir _dir, string _name, IntegerType *_type,
                          bool _is_static);
  DataType *GetType();
  bool IsScalar();
  bool HasDefaultValue();
  BitConstant GetDefaultValue();
  void SetDefaultValue(BitConstant defval);
  void Synthesise(SynthContext &sc);
  // Static variables only
  EvaluatorVariable *GetChildByName(string name);
  vector<EvaluatorVariable *> GetAllChildren();
  void HandleWrite(Evaluator *genst, EvalObject *value);

private:
  IntegerType *type;
  bool hasDefaultValue = false;
  BitConstant defaultValue;
  // Static variables only
  bool is_static = false;
  ScalarEvaluatorVariable *write_enable = nullptr;
  ScalarEvaluatorVariable *written_value = nullptr;
};

class ArrayEvaluatorVariable : public EvaluatorVariable {
public:
  ArrayEvaluatorVariable(VariableDir _dir, string _name, ArrayType *_type,
                         bool _is_static);
  DataType *GetType();
  bool IsScalar();

  vector<EvaluatorVariable *> GetAllChildren();
  vector<EvaluatorVariable *> GetArrayChildren();
  void SetBitOffset(int _bitoffset);

  EvalObject *HandleRead(Evaluator *genst);
  void HandleWrite(Evaluator *genst, EvalObject *value);

private:
  ArrayType *type;
  vector<EvaluatorVariable *> arrayItems;
};

class StructureEvaluatorVariable : public EvaluatorVariable {
public:
  StructureEvaluatorVariable(VariableDir _dir, string _name,
                             StructureType *_type, bool _is_static);
  DataType *GetType();
  bool IsScalar();

  vector<EvaluatorVariable *> GetAllChildren();
  EvaluatorVariable *GetChildByName(string name);
  void SetBitOffset(int _bitoffset);

  EvalObject *HandleRead(Evaluator *genst);
  void HandleWrite(Evaluator *genst, EvalObject *value);

private:
  StructureType *type;
  vector<EvaluatorVariable *> structItems;
};

class ExternalMemoryEvaluatorVariable : public EvaluatorVariable {
public:
  ExternalMemoryEvaluatorVariable(VariableDir _dir, string _name,
                                  RAMType *_type);
  DataType *GetType();
  bool IsScalar();

  vector<EvaluatorVariable *> GetAllChildren();
  EvaluatorVariable *GetChildByName(string name);
  MemoryDeviceParameters GetMemoryParams();

  bool IsNonTrivialArrayAccess();
  EvalObject *HandleSubscriptedRead(Evaluator *genst,
                                    vector<EvalObject *> index);
  void HandleSubscriptedWrite(Evaluator *genst, vector<EvalObject *> index,
                              EvalObject *value);

  EvalObject *HandleRead(Evaluator *genst);
  void HandleWrite(Evaluator *genst, EvalObject *value);

private:
  RAMType *type;
  map<string, ScalarEvaluatorVariable *> ports;
};

class StreamEvaluatorVariable : public EvaluatorVariable {
public:
  StreamEvaluatorVariable(VariableDir _dir, string _name, StreamType *_type);
  DataType *GetType();
  bool IsScalar();

  vector<EvaluatorVariable *> GetAllChildren();
  EvaluatorVariable *GetChildByName(string name);
  vector<EvaluatorVariable *> GetArrayChildren();
  void HandlePush(Evaluator *genst, EvalObject *value);

  void HandleWrite(Evaluator *genst, EvalObject *value);

private:
  StreamType *type;
  vector<EvaluatorVariable *> streamWindow;
  EvaluatorVariable *written_value;
  ScalarEvaluatorVariable *write_enable;
};
}
