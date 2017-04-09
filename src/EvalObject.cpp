#include "EvalObject.hpp"
#include "BitConstant.hpp"
#include "Evaluator.hpp"
#include "EvaluatorState.hpp"
#include "Operations.hpp"
#include "Util.hpp"
#include "hdl/HDLCoreDevices.hpp"
#include <algorithm>
#include <iterator>
#include <stdexcept>
using namespace std;
namespace RapidHLS {
/* EvalObject base */
EvalObject::EvalObject() { base_id = GetUniqueID(); };

string EvalObject::GetID() { return "eval_" + to_string(base_id); };

bool EvalObject::HasConstantValue(Evaluator *state) { return false; };

EvalObject *EvalObject::GetConstantValue(Evaluator *state) {
  throw eval_error("===" + GetID() + "=== not a constant");
};

BitConstant EvalObject::GetScalarConstValue(Evaluator *state) {
  if (dynamic_cast<IntegerType *>(GetDataType(state)) == nullptr) {
    throw eval_error("===" + GetID() + "=== not a valid scalar constant");
  } else {
    return GetConstantValue(state)->GetScalarConstValue(state);
  }
}

EvalObject *
EvalObject::ApplyArraySubscriptRead(Evaluator *state,
                                    vector<EvalObject *> subscript) {
  throw eval_error("===" + GetID() + "=== not an array or array-like type");
};

void EvalObject::ApplyArraySubscriptWrite(Evaluator *state,
                                          vector<EvalObject *> subscript,
                                          EvalObject *value) {
  throw eval_error("===" + GetID() + "=== not an array or array-like type");
};

EvalObject *EvalObject::GetStructureMember(Evaluator *state, string name) {
  throw eval_error("===" + GetID() + "=== not of structure type");
};

void EvalObject::AssignStructureMember(Evaluator *state, string name,
                                       EvalObject *value) {
  throw eval_error("===" + GetID() + "=== not of structure type");
}

EvalObject *EvalObject::ApplyToState(Evaluator *state) { return this; };

void EvalObject::AssignValue(Evaluator *state, EvalObject *value) {
  throw eval_error("===" + GetID() + "=== cannot be assigned to");
};

vector<EvalObject *> EvalObject::GetOperands() {
  return vector<EvalObject *>{};
};

EvalObject *EvalObject::GetValue(Evaluator *state) { return EvalNull; }

bool EvalObject::CanPushInto() { return false; }

EvalObject *EvalObject::ApplyPushInto(Evaluator *state, EvalObject *value) {
  return EvalNull;
}

void EvalObject::Synthesise(Evaluator *state, HDLGen::HDLDesign *design,
                            HDLGen::HDLSignal *outputNet) {
  throw eval_error("===" + GetID() + "=== cannot be synthesised to HDL");
}

/* EvalVariable */
EvalVariable::EvalVariable(EvaluatorVariable *_var) : EvalObject(), var(_var){};

string EvalVariable::GetID() {
  return "eval_var_" + var->name + "_" + to_string(base_id);
}

DataType *EvalVariable::GetDataType(Evaluator *state) {
  return var->GetType();
};

bool EvalVariable::HasConstantValue(Evaluator *state) {
  if (var->GetDir().is_input) {
    return false;
  } else {
    try {
      GetConstantValue(state);
      return true;
    } catch (eval_error &) {
      return false;
    }
  }
}

EvalObject *EvalVariable::GetConstantValue(Evaluator *state) {
  if (var->GetDir().is_input) {
    throw eval_error("input (top-level or internal) variable ===" + var->name +
                     "=== cannot be used as a constant");
  } else {
    return var->HandleRead(state)->GetConstantValue(state);
  }
};

EvalObject *
EvalVariable::ApplyArraySubscriptRead(Evaluator *state,
                                      vector<EvalObject *> subscript) {
  if (var->IsNonTrivialArrayAccess()) {
    return var->HandleSubscriptedRead(state, subscript);
  } else {
    // check dimensions - note if multiple are specified this is because a
    // single subscript in the form [x,y] was used
    if (subscript.size() == var->GetType()->GetDimensions().size()) {
      bool isConst = true;
      int offset = 0;
      int lastDim = 1;
      for (int i = 0; i < var->GetType()->GetDimensions().size(); i++) {
        if (!subscript[i]->HasConstantValue(state)) {
          isConst = false;
          break;
        }
        offset *= lastDim;
        int indval = subscript[i]->GetScalarConstValue(state).intval();
        lastDim = var->GetType()->GetDimensions()[i];
        if (indval < lastDim) {
          offset += indval;
        } else {
          throw eval_error("array index out of bounds for variable ===" +
                           var->name + "===");
        }
      }
      if (isConst) {
        return new EvalVariable(var->GetArrayChildren()[offset]);
      } else {
        throw eval_error("non-constant array indices are not yet implemented");
      }
    } else {
      throw eval_error("dimensionality mismatch for variable ===" + var->name +
                       "===");
    }
  }
}

void EvalVariable::ApplyArraySubscriptWrite(Evaluator *state,
                                            vector<EvalObject *> subscript,
                                            EvalObject *value) {
  if (var->IsNonTrivialArrayAccess()) {
    var->HandleSubscriptedRead(state, subscript);
  } else {
    // check dimensions - note if multiple are specified this is because a
    // single subscript in the form [x,y] was used
    // at some point this should probably be moved to a common function
    if (subscript.size() == var->GetType()->GetDimensions().size()) {
      bool isConst = true;
      int offset = 0;
      int lastDim = 1;
      for (int i = 0; i < var->GetType()->GetDimensions().size(); i++) {
        if (!subscript[i]->HasConstantValue(state)) {
          isConst = false;
          break;
        }
        offset *= lastDim;
        int indval = subscript[i]->GetScalarConstValue(state).intval();
        lastDim = var->GetType()->GetDimensions()[i];
        if (indval < lastDim) {
          offset += indval;
        } else {
          throw eval_error("array index out of bounds for variable ===" +
                           var->name + "===");
        }
      }
      if (isConst) {
        var->GetArrayChildren()[offset]->HandleWrite(state, value);
      } else {
        throw eval_error("non-constant array indices are not yet implemented");
      }
    } else {
      throw eval_error("dimensionality mismatch for variable ===" + var->name +
                       "===");
    }
  }
}

EvalObject *EvalVariable::GetStructureMember(Evaluator *state, string name) {
  return new EvalVariable(var->GetChildByName(name));
}
void EvalVariable::AssignStructureMember(Evaluator *state, string name,
                                         EvalObject *value) {
  var->GetChildByName(name)->HandleWrite(state, value);
}
void EvalVariable::AssignValue(Evaluator *state, EvalObject *value) {
  var->HandleWrite(state, value);
}
EvalObject *EvalVariable::GetValue(Evaluator *state) {
  return var->HandleRead(state);
}

/*EvalConstant*/
EvalConstant::EvalConstant(BitConstant _val) : EvalObject(), val(_val){};

string EvalConstant::GetID() { return "const_" + to_string(base_id); };

bool EvalConstant::HasConstantValue(Evaluator *state) { return true; }

EvalObject *EvalConstant::GetConstantValue(Evaluator *state) { return this; };

DataType *EvalConstant::GetDataType(Evaluator *state) {
  return new IntegerType(val.bits.size(), val.is_signed);
}

BitConstant EvalConstant::GetScalarConstValue(Evaluator *state) { return val; };

void EvalConstant::Synthesise(Evaluator *state, HDLGen::HDLDesign *design,
                              HDLGen::HDLSignal *outputNet) {
  design->AddDevice(new HDLGen::ConstantHDLDevice(val, outputNet));
}

EvalArray::EvalArray(ArrayType *_arrType, const vector<EvalObject *> &_items)
    : EvalObject(), arrType(_arrType), items(_items){};

string EvalArray::GetID() { return "temp_array_" + to_string(base_id); };

bool EvalArray::HasConstantValue(Evaluator *state) {
  return all_of(items.begin(), items.end(), [state](EvalObject *itm) {
    return itm->HasConstantValue(state);
  });
}

DataType *EvalArray::GetDataType(Evaluator *state) { return arrType; }

EvalObject *EvalArray::GetConstantValue(Evaluator *state) {
  vector<EvalObject *> constItems;
  transform(items.begin(), items.end(), back_inserter(constItems),
            [state](EvalObject *itm) { return itm->GetConstantValue(state); });
  return new EvalArray(arrType, constItems);
}

EvalObject *EvalArray::ApplyArraySubscriptRead(Evaluator *state,
                                               vector<EvalObject *> subscript) {
  int offset = 0;
  for (int i = 0; i < subscript.size(); i++) {
    if (i > 0)
      offset *= arrType->GetDimensions().at(i - 1);
    offset += subscript[i]
                  ->GetConstantValue(state)
                  ->GetScalarConstValue(state)
                  .intval();
  }
  return items.at(offset);
}

void EvalArray::ApplyArraySubscriptWrite(Evaluator *state,
                                         vector<EvalObject *> subscript,
                                         EvalObject *value) {
  int offset = 0;
  for (int i = 0; i < subscript.size(); i++) {
    if (i > 0)
      offset *= arrType->GetDimensions().at(i - 1);
    offset += subscript[i]
                  ->GetConstantValue(state)
                  ->GetScalarConstValue(state)
                  .intval();
  }
  items.at(offset) = value;
}

vector<EvalObject *> EvalArray::GetOperands() { return items; }

/*EvalStruct*/
EvalStruct::EvalStruct(StructureType *_structType,
                       const map<string, EvalObject *> _items)
    : EvalObject(), structType(_structType), items(_items){};

string EvalStruct::GetID() { return "temp_struct_" + to_string(base_id); };

bool EvalStruct::HasConstantValue(Evaluator *state) {
  return all_of(items.begin(), items.end(),
                [=](pair<const string, EvalObject *> &p) {
                  return p.second->HasConstantValue(state);
                });
}

DataType *EvalStruct::GetDataType(Evaluator *state) { return structType; };

EvalObject *EvalStruct::GetConstantValue(Evaluator *state) {
  map<string, EvalObject *> constItems = items;
  for_each(constItems.begin(), constItems.end(),
           [=](pair<const string, EvalObject *> &p) {
             p.second = p.second->GetConstantValue(state);
           });
  return new EvalStruct(structType, constItems);
}

EvalObject *EvalStruct::GetStructureMember(Evaluator *state, string name) {
  try {
    return items.at(name);
  } catch (out_of_range) {
    throw eval_error("structure type ===" + structType->structName +
                     "=== does not contain member ===" + name + "===");
  }
}

void EvalStruct::AssignStructureMember(Evaluator *state, string name,
                                       EvalObject *value) {
  throw eval_error("cannot assign to temporary struct");
}

vector<EvalObject *> EvalStruct::GetOperands() {
  vector<EvalObject *> operands;
  transform(items.begin(), items.end(), back_inserter(operands),
            [](pair<const string, EvalObject *> p) { return p.second; });
  return operands;
}

EvalCast::EvalCast(IntegerType *_castTo, EvalObject *_operand)
    : EvalObject(), castTo(_castTo), operand(_operand){};

string EvalCast::GetID() { return "cast_" + to_string(base_id); }

DataType *EvalCast::GetDataType(Evaluator *state) { return castTo; };

bool EvalCast::HasConstantValue(Evaluator *state) {
  return operand->HasConstantValue(state);
};

EvalObject *EvalCast::GetConstantValue(Evaluator *state) {
  return new EvalCast(castTo, operand->GetConstantValue(state));
}

BitConstant EvalCast::GetScalarConstValue(Evaluator *state) {
  return operand->GetScalarConstValue(state).cast(castTo->width,
                                                  castTo->is_signed);
}

vector<EvalObject *> EvalCast::GetOperands() { return {operand}; }

EvalObject *EvalCast::GetValue(Evaluator *state) {
  return new EvalCast(castTo, operand->GetValue(state));
}

EvalBasicOperation::EvalBasicOperation(OperationType _type,
                                       vector<EvalObject *> _operands)
    : EvalObject(), type(_type), operands(_operands){};

string EvalBasicOperation::GetID() { return "oper_" + to_string(base_id); };

DataType *EvalBasicOperation::GetDataType(Evaluator *state) {
  if (NonNumericAllowed()) {
    return operands[1]->GetDataType(state);
  } else {
    vector<IntegerType *> intTypes;
    vector<int> widths;
    vector<BitConstant *> cnstVals;

    transform(operands.begin(), operands.end(), back_inserter(intTypes),
              [state](EvalObject *o) {
                return dynamic_cast<IntegerType *>(o->GetDataType(state));
              });

    if (find(intTypes.begin(), intTypes.end(), nullptr) != intTypes.end()) {
      throw eval_error("all operands of operator" +
                       LookupOperation(type)->token +
                       " must be numeric and scalar");
    }

    transform(intTypes.begin(), intTypes.end(), back_inserter(widths),
              [](IntegerType *i) { return i->width; });

    transform(operands.begin(), operands.end(), back_inserter(cnstVals),
              [state](EvalObject *o) -> BitConstant * {
                if (o->HasConstantValue(state)) {
                  return new BitConstant(o->GetScalarConstValue(state));
                } else {
                  return nullptr;
                }
              });

    bool result_signed = any_of(intTypes.begin(), intTypes.end(),
                                [](IntegerType *t) { return t->is_signed; });

    return new IntegerType(GetResultWidth(widths, type, cnstVals),
                           result_signed);
  }
};

bool EvalBasicOperation::HasConstantValue(Evaluator *state) {
  try {
    GetScalarConstValue(state);
    return true;
  } catch (runtime_error &r) {
    return false;
  }
};

EvalObject *EvalBasicOperation::GetConstantValue(Evaluator *state) {
  if (LookupOperation(type)->is_assignment)
    throw eval_error("assignment type operation does not have const value");
  vector<BitConstant> constOperands;
  transform(operands.begin(), operands.end(), back_inserter(constOperands),
            [state](EvalObject *o) { return o->GetScalarConstValue(state); });
  return new EvalConstant(PerformConstOperation(constOperands, type));
};

EvalObject *EvalBasicOperation::ApplyToState(Evaluator *state) {
  // Perform ApplyToState on all operands
  vector<EvalObject *> apOperands;
  transform(operands.begin(), operands.end(), back_inserter(apOperands),
            [state](EvalObject *o) { return o->ApplyToState(state); });

  if (NonNumericAllowed()) {
    EvalObject *value = apOperands[1]->GetValue(state);
    if (type == B_LS) {
      // push
      apOperands[0]->ApplyPushInto(state, value);
    } else if (type == B_ASSIGN) {
      // simple assignment
      apOperands[0]->AssignValue(state, value);
    } else {
      throw eval_error("invalid operand");
    }
    return value;
  } else {
    EvalObject *result = GetResult(state, apOperands);

    if (LookupOperation(type)->is_assignment) {
      // for everything except postfix increment and decrement, the assigned
      // value is the same as the result value
      if (type == U_POSTINC) {
        apOperands[0]->AssignValue(state,
                                   GetResult(state, apOperands, U_PREINC));
      } else if (type == U_POSTDEC) {
        apOperands[0]->AssignValue(state,
                                   GetResult(state, apOperands, U_PREDEC));
      } else {
        apOperands[0]->AssignValue(state, result);
      }
    }

    return result;
  }
};

vector<EvalObject *> EvalBasicOperation::GetOperands() { return operands; };

EvalObject *EvalBasicOperation::GetValue(Evaluator *state) {
  return GetResult(state, operands, type);
};

bool EvalBasicOperation::NonNumericAllowed() {
  if (type == B_ASSIGN) {
    return true;
  } else if ((type == B_LS) && (operands[0]->CanPushInto())) {
    return true;
  } else {
    return false;
  }
}

EvalObject *
EvalBasicOperation::GetResult(Evaluator *state,
                              const vector<EvalObject *> &operands) {
  return GetResult(state, operands, type);
}

EvalObject *EvalBasicOperation::GetResult(Evaluator *state,
                                          const vector<EvalObject *> &operands,
                                          OperationType type) {
  if (NonNumericAllowed()) {
    return operands[1]->GetValue(state);
  } else {
    vector<EvalObject *> operandValues;
    transform(operands.begin(), operands.end(), back_inserter(operandValues),
              [state](EvalObject *o) { return o->GetValue(state); });
    if (LookupOperation(type)->is_assignment) {
      switch (type) {
      case B_ASSIGN:
        return operandValues[1];
      case B_PLUSEQ:
        return GetResult(state, operandValues, B_ADD);
      case B_MINUSEQ:
        return GetResult(state, operandValues, B_SUB);
      case B_MULEQ:
        return GetResult(state, operandValues, B_MUL);
      case B_DIVEQ:
        return GetResult(state, operandValues, B_DIV);
      case B_MODEQ:
        return GetResult(state, operandValues, B_MOD);
      case B_OREQ:
        return GetResult(state, operandValues, B_BWOR);
      case B_ANDEQ:
        return GetResult(state, operandValues, B_BWAND);
      case B_XOREQ:
        return GetResult(state, operandValues, B_BWXOR);
      case B_LSEQ:
        return GetResult(state, operandValues, B_LS);
      case B_RSEQ:
        return GetResult(state, operandValues, B_RS);
      case U_POSTINC:
      case U_POSTDEC:
        return operandValues[0];
      case U_PREINC:
        return new EvalBasicOperation(
            B_ADD, vector<EvalObject *>{operandValues[0],
                                        new EvalConstant(BitConstant(1))});
      case U_PREDEC:
        return new EvalBasicOperation(
            B_SUB, vector<EvalObject *>{operandValues[0],
                                        new EvalConstant(BitConstant(1))});
      default:
        throw eval_error("unknown assignment type operation");
      };
    } else {
      return new EvalBasicOperation(type, operands);
    }
  }
}

void EvalBasicOperation::Synthesise(Evaluator *state, HDLGen::HDLDesign *design,
                                    HDLGen::HDLSignal *outputNet) {
  vector<HDLGen::HDLSignal *> operandSigs;

  transform(operands.begin(), operands.end(), back_inserter(operandSigs),
            [design, state](EvalObject *op) {
              HDLGen::HDLSignal *res = design->CreateTempSignal(
                  op->GetDataType(state)->GetHDLType());
              op->Synthesise(state, design, res);
              return res;
            });

  design->AddDevice(
      new HDLGen::OperationHDLDevice(type, operandSigs, outputNet));
}

EvalArrayAccess::EvalArrayAccess(EvalObject *_base, vector<EvalObject *> _index)
    : EvalObject(), base(_base), index(_index){};

string EvalArrayAccess::GetID() { return "array_access_" + to_string(base_id); }

DataType *EvalArrayAccess::GetDataType(Evaluator *state) {
  return base->GetDataType(state)->GetBaseType();
}

bool EvalArrayAccess::HasConstantValue(Evaluator *state) {
  return base->HasConstantValue(state) &&
         all_of(index.begin(), index.end(),
                [state](EvalObject *i) { return i->HasConstantValue(state); });
}

EvalObject *EvalArrayAccess::GetConstantValue(Evaluator *state) {
  vector<EvalObject *> constIndex;
  transform(index.begin(), index.end(), back_inserter(constIndex),
            [state](EvalObject *i) { return i->GetConstantValue(state); });
  return base->GetConstantValue(state)->ApplyArraySubscriptRead(state,
                                                                constIndex);
}

EvalObject *
EvalArrayAccess::ApplyArraySubscriptRead(Evaluator *state,
                                         vector<EvalObject *> subscript) {
  return base->ApplyArraySubscriptRead(state, index)
      ->ApplyArraySubscriptRead(state, subscript);
}

void EvalArrayAccess::ApplyArraySubscriptWrite(Evaluator *state,
                                               vector<EvalObject *> subscript,
                                               EvalObject *value) {
  base->ApplyArraySubscriptRead(state, index)
      ->ApplyArraySubscriptWrite(state, subscript, value);
}

EvalObject *EvalArrayAccess::GetStructureMember(Evaluator *state, string name) {
  return base->ApplyArraySubscriptRead(state, index)
      ->GetStructureMember(state, name);
}
void EvalArrayAccess::AssignStructureMember(Evaluator *state, string name,
                                            EvalObject *value) {
  base->ApplyArraySubscriptRead(state, index)
      ->AssignStructureMember(state, name, value);
}

EvalObject *EvalArrayAccess::ApplyToState(Evaluator *state) {
  for_each(index.begin(), index.end(),
           [state](EvalObject *i) { i->ApplyToState(state); });
  base->ApplyToState(state);
  return GetValue(state);
}

void EvalArrayAccess::AssignValue(Evaluator *state, EvalObject *value) {
  base->ApplyArraySubscriptWrite(state, index, value);
}

vector<EvalObject *> EvalArrayAccess::GetOperands() {
  vector<EvalObject *> operands;
  operands.push_back(base);
  copy(index.begin(), index.end(), back_inserter(operands));
  return operands;
}

EvalObject *EvalArrayAccess::GetValue(Evaluator *state) {
  vector<EvalObject *> transformedIndex;
  transform(index.begin(), index.end(), back_inserter(transformedIndex),
            [state](EvalObject *i) { return i->GetValue(state); });
  return base->ApplyArraySubscriptRead(state, transformedIndex)
      ->GetValue(state);
}

EvalStructAccess::EvalStructAccess(EvalObject *_base, string _member)
    : EvalObject(), base(_base), member(_member){};

DataType *EvalStructAccess::GetDataType(Evaluator *state) {
  return base->GetDataType(state)->GetMemberType(member);
}

bool EvalStructAccess::HasConstantValue(Evaluator *state) {
  return base->HasConstantValue(state);
}

EvalObject *EvalStructAccess::GetConstantValue(Evaluator *state) {
  return base->GetStructureMember(state, member)->GetConstantValue(state);
}

EvalObject *
EvalStructAccess::ApplyArraySubscriptRead(Evaluator *state,
                                          vector<EvalObject *> subscript) {
  return base->GetStructureMember(state, member)
      ->ApplyArraySubscriptRead(state, subscript);
}
void EvalStructAccess::ApplyArraySubscriptWrite(Evaluator *state,
                                                vector<EvalObject *> subscript,
                                                EvalObject *value) {
  base->GetStructureMember(state, member)
      ->ApplyArraySubscriptWrite(state, subscript, value);
};
EvalObject *EvalStructAccess::GetStructureMember(Evaluator *state,
                                                 string name) {
  return base->GetStructureMember(state, member)
      ->GetStructureMember(state, name);
}
void EvalStructAccess::AssignStructureMember(Evaluator *state, string name,
                                             EvalObject *value) {
  base->GetStructureMember(state, member)
      ->AssignStructureMember(state, name, value);
}

EvalObject *EvalStructAccess::ApplyToState(Evaluator *state) {
  return base->ApplyToState(state)->GetStructureMember(state, member);
}

void EvalStructAccess::AssignValue(Evaluator *state, EvalObject *value) {
  return base->AssignStructureMember(state, member, value);
}

vector<EvalObject *> EvalStructAccess::GetOperands() { return {base}; }

EvalObject *EvalStructAccess::GetValue(Evaluator *state) {
  return base->GetStructureMember(state, member)->GetValue(state);
}

EvalRegister::EvalRegister(EvalObject *_input) : EvalObject(), input(_input){};

string EvalRegister::GetID() { return "reg_" + to_string(base_id); }

DataType *EvalRegister::GetDataType(Evaluator *state) {
  return input->GetDataType(state);
}

bool EvalRegister::HasConstantValue(Evaluator *state) {
  return input->HasConstantValue(state);
}

EvalObject *EvalRegister::GetConstantValue(Evaluator *state) {
  return input->GetConstantValue(state);
}

EvalObject *EvalRegister::ApplyToState(Evaluator *state) {
  return new EvalRegister(input->ApplyToState(state));
}
vector<EvalObject *> EvalRegister::GetOperands() { return {input}; }

EvalObject *EvalRegister::GetValue(Evaluator *state) {
  return new EvalRegister(input->GetValue(state));
}

EvalNull_class::EvalNull_class() : EvalObject(){};

string EvalNull_class::GetID() { return "<null>"; };
DataType *EvalNull_class::GetDataType(Evaluator *state) {
  throw eval_error(nullMessage);
}
bool EvalNull_class::HasConstantValue(Evaluator *state) { return false; };
EvalObject *EvalNull_class::GetConstantValue(Evaluator *state) {
  throw eval_error(nullMessage);
}
EvalObject *
EvalNull_class::ApplyArraySubscriptRead(Evaluator *state,
                                        vector<EvalObject *> subscript) {
  throw eval_error(nullMessage);
}
void EvalNull_class::ApplyArraySubscriptWrite(Evaluator *state,
                                              vector<EvalObject *> subscript,
                                              EvalObject *value) {
  throw eval_error(nullMessage);
}
EvalObject *EvalNull_class::GetStructureMember(Evaluator *state, string name) {
  throw eval_error(nullMessage);
}
void EvalNull_class::AssignStructureMember(Evaluator *state, string name,
                                           EvalObject *value) {
  throw eval_error(nullMessage);
}
EvalObject *EvalNull_class::ApplyToState(Evaluator *state) {
  throw eval_error(nullMessage);
}
void EvalNull_class::AssignValue(Evaluator *state, EvalObject *value) {
  throw eval_error(nullMessage);
}
vector<EvalObject *> EvalNull_class::GetOperands() {
  throw eval_error(nullMessage);
}
EvalObject *EvalNull_class::GetValue(Evaluator *state) {
  throw eval_error(nullMessage);
}

EvalNull_class EvalNull_obj;
EvalNull_class *EvalNull = &EvalNull_obj;
}
