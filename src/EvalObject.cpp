#include "EvalObject.hpp"
#include "BitConstant.hpp"
#include "Evaluator.hpp"
#include "EvaluatorState.hpp"
#include "Operations.hpp"
#include "Util.hpp"
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
    return state->GetVariableValue(var)->GetConstantValue(state);
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
  return state->GetVariableValue(var);
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

// TODO: EvalArray

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
}
