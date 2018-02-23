#include "HDLCoreDevices.hpp"
#include "HDLDevicePort.hpp"
#include "HDLSignal.hpp"

#include <algorithm>
using namespace std;

namespace RapidHLS {
namespace HDLGen {
OperationHDLDevice::OperationHDLDevice(OperationType _oper,
                                       const vector<HDLSignal *> &inputs,
                                       HDLSignal *output)
    : oper(_oper) {
  inst_name = "basic_op_" + to_string(serial++);
  int input_num = 1;
  for (auto input : inputs) {
    ports.push_back(new HDLDevicePort("input_" + to_string(input_num), this,
                                      input->sigType, input,
                                      PortDirection::Input));
    input_num++;
  }
  ports.push_back(new HDLDevicePort("output", this, output->sigType, output,
                                    PortDirection::Output));
}

string OperationHDLDevice::GetInstanceName() { return inst_name; }

vector<HDLDevicePort *> &OperationHDLDevice::GetPorts() { return ports; };

vector<string> OperationHDLDevice::GetVHDLDeps() {
  return vector<string>{"ieee.std_logic_1164.all", "ieee.numeric_std.all"};
}

void OperationHDLDevice::GenerateVHDLPrefix(ostream &vhdl) {
  // nothing needed here
}

void OperationHDLDevice::GenerateVHDL(ostream &vhdl) {
  string value = "";
  int width = 0;
  bool is_signed = false;
  vector<string> operands;
  // Work out max width and signedness
  for (int i = 0; i < ports.size() - 1; i++) {
    operands.push_back(ports.at(i)->connectedNet->name);
    HDLPortType *type = ports.at(i)->type;
    width = max(width, type->GetWidth());
    is_signed |= type->IsSigned();
  }
  // Add/sub extend width by 1 to guarantee no overflow
  if ((oper == OperationType::B_ADD) || (oper == OperationType::B_SUB)) {
    width += 1;
  }
  // Multiplication and comparison keeps operand types to allow signed/unsigned
  // and mixed
  // width multiplies/compares, everything else casts to a common type
  for (int i = 0; i < ports.size() - 1; i++) {
    HDLPortType *type = ports.at(i)->type;
    if ((oper == OperationType::B_MUL) || (oper == OperationType::B_EQ) ||
        (oper == OperationType::B_NEQ) || (oper == OperationType::B_GT) ||
        (oper == OperationType::B_LT) || (oper == OperationType::B_GTE) ||
        (oper == OperationType::B_LTE)) {
      operands.at(i) = NumericPortType(type->GetWidth(), type->IsSigned())
                           .VHDLCastFrom(type, operands.at(i));
    } else {
      operands.at(i) =
          NumericPortType(width, is_signed).VHDLCastFrom(type, operands.at(i));
    }
  }
  // Logical operations use a vhdl when/esle construct so can't be wrapped in a
  // cast
  bool is_logical = false;

  // One and zero cast to result type for use in logical operations
  NumericPortType logConstType(1, false);
  string zero = ports.back()->connectedNet->sigType->VHDLCastFrom(
      &logConstType, "to_unsigned(0, 1)");
  string one = ports.back()->connectedNet->sigType->VHDLCastFrom(
      &logConstType, "to_unsigned(1, 1)");

  switch (oper) {
  case OperationType::B_ADD:
    value = operands.at(0) + " + " + operands.at(1);
    break;
  case OperationType::B_SUB:
    value = operands.at(0) + " - " + operands.at(1);
    break;
  case OperationType::B_MUL:
    value = operands.at(0) + " * " + operands.at(1);
    break;
  case OperationType::B_BWAND:
    value = operands.at(0) + " and " + operands.at(1);
    break;
  case OperationType::B_BWOR:
    value = operands.at(0) + " or " + operands.at(1);
    break;
  case OperationType::B_BWXOR:
    value = operands.at(0) + " xor " + operands.at(1);
    break;
  case OperationType::U_MINUS:
    value = "0 - " + operands.at(0);
    break;
  case OperationType::U_BWNOT:
    value = "not " + operands.at(0);
    break;
  case OperationType::B_EQ:
    value = one + " when " + operands.at(0) + " = " + operands.at(1) +
            " else " + zero;
    break;
  case OperationType::B_NEQ:
    value = one + " when " + operands.at(0) + " /= " + operands.at(1) +
            " else " + zero;
    break;
  case OperationType::B_LT:
    value = one + " when " + operands.at(0) + " < " + operands.at(1) +
            " else " + zero;
    break;
  case OperationType::B_GT:
    value = one + " when " + operands.at(0) + " > " + operands.at(1) +
            " else " + zero;
    break;
  case OperationType::B_LTE:
    value = one + " when " + operands.at(0) + " <= " + operands.at(1) +
            " else " + zero;
    break;
  case OperationType::B_GTE:
    value = one + " when " + operands.at(0) + " >= " + operands.at(1) +
            " else " + zero;
    break;
  case OperationType::B_LAND:
    value = one + " when " + operands.at(0) + " /= 0 and " + operands.at(1) +
            " /= 0 else " + zero;
    break;
  case OperationType::B_LOR:
    value = one + " when " + operands.at(0) + " /= 0 or " + operands.at(1) +
            " /= 0 else " + zero;
    break;
  case OperationType::U_LNOT:
    value = one + " when " + operands.at(0) + " == 0 else " + zero;
    break;
  case OperationType::B_LS:
    value = "shift_left(" + operands.at(0) + ", to_integer(" + operands.at(1) +
            "))";
    break;
  case OperationType::B_RS:
    value = "shift_right(" + operands.at(0) + ", to_integer(" + operands.at(1) +
            "))";
    break;
  default:
    throw runtime_error("operation type not supported in HDL");
  }

  NumericPortType resType(width, is_signed);
  vhdl << "\t" << ports.back()->connectedNet->name << " <= ";
  if (is_logical) {
    vhdl << value;
  } else {
    vhdl << ports.back()->connectedNet->sigType->VHDLCastFrom(&resType, value);
  }
  vhdl << ";" << endl;
}

void OperationHDLDevice::AnnotateTiming(DeviceTiming *model) {
  HDLTimingValue<double> inp_delay =
      (*max_element(ports.begin(), ports.end() - 1,
                    [](HDLDevicePort *a, HDLDevicePort *b) {
                      return a->connectedNet->timing_delay <
                             b->connectedNet->timing_delay;
                    }))
          ->connectedNet->timing_delay;
  // TODO: use model to calculate device delay
  double dev_delay = 0;
  ports.back()->connectedNet->timing_delay = inp_delay + dev_delay;
  // TODO: should we propogate clock domains?
}

void OperationHDLDevice::AnnotateLatency(DeviceTiming *model) {
  HDLTimingValue<int> inp_latency =
      (*max_element(ports.begin(), ports.end() - 1,
                    [](HDLDevicePort *a, HDLDevicePort *b) {
                      return a->connectedNet->pipeline_latency <
                             b->connectedNet->pipeline_latency;
                    }))
          ->connectedNet->pipeline_latency;
  ports.back()->connectedNet->pipeline_latency = inp_latency;
}

OperationHDLDevice::~OperationHDLDevice() {
  for_each(ports.begin(), ports.end(), [](HDLDevicePort *p) { delete p; });
}

int OperationHDLDevice::serial = 0;

RegisterHDLDevice::RegisterHDLDevice(HDLSignal *d, HDLSignal *clk, HDLSignal *q,
                                     HDLSignal *en, HDLSignal *rst,
                                     bool _is_pipeline)
    : is_pipeline(_is_pipeline) {
  inst_name = "reg_" + to_string(serial++);
  ports.push_back(
      new HDLDevicePort("d", this, d->sigType, d, PortDirection::Input));
  ports.push_back(
      new HDLDevicePort("clk", this, clk->sigType, clk, PortDirection::Input));
  ports.push_back(
      new HDLDevicePort("q", this, q->sigType, q, PortDirection::Output));
  ports.push_back(
      new HDLDevicePort("en", this, en->sigType, en, PortDirection::Input));
  ports.push_back(
      new HDLDevicePort("rst", this, rst->sigType, rst, PortDirection::Input));
}

string RegisterHDLDevice::GetInstanceName() { return inst_name; }

vector<HDLDevicePort *> &RegisterHDLDevice::GetPorts() { return ports; };

vector<string> RegisterHDLDevice::GetVHDLDeps() {
  return vector<string>{"ieee.std_logic_1164.all", "ieee.numeric_std.all"};
}

void RegisterHDLDevice::GenerateVHDLPrefix(ostream &vhdl) {}

void RegisterHDLDevice::GenerateVHDL(ostream &vhdl) {
  string clksig = ports.at(1)->connectedNet->name;
  vhdl << "\tprocess(" << clksig << ")" << endl;
  vhdl << "\tbegin" << endl;
  vhdl << "\t\tif rising_edge(clksig) then" << endl;
  vhdl << "\t\t\tif " << ports.at(4)->connectedNet->name << " = '1' then"
       << endl;
  vhdl << "\t\t\t\t" << ports.at(2)->connectedNet->name
       << " <= " << ports.at(2)->type->GetZero() << ";" << endl;
  vhdl << "\t\t\telsif " << ports.at(3)->connectedNet->name << " = '1' then"
       << endl;
  vhdl << "\t\t\t\t" << ports.at(2)->connectedNet->name << " <= "
       << ports.at(2)->type->VHDLCastFrom(ports.at(0)->type,
                                          ports.at(0)->connectedNet->name)
       << ";" << endl;
  vhdl << "\t\t\tend if;" << endl;
  vhdl << "\t\tend if;" << endl;
  vhdl << "\tend process;" << endl << endl;
}

void RegisterHDLDevice::AnnotateTiming(DeviceTiming *model) {
  ports.at(2)->connectedNet->timing_delay = HDLTimingValue<double>(
      ports.at(1)->connectedNet, model->GetFFPropogationDelay());
}

void RegisterHDLDevice::AnnotateLatency(DeviceTiming *model) {
  ports.at(2)->connectedNet->pipeline_latency =
      ports.at(0)->connectedNet->pipeline_latency + (is_pipeline ? 1 : 0);
}

RegisterHDLDevice::~RegisterHDLDevice() {
  for_each(ports.begin(), ports.end(), [](HDLDevicePort *p) { delete p; });
}

int RegisterHDLDevice::serial = 0;

ConstantHDLDevice::ConstantHDLDevice(BitConstant _value, HDLSignal *output)
    : value(_value) {
  inst_name = "const_" + to_string(serial++);
  ports.push_back(new HDLDevicePort("output", this, output->sigType, output,
                                    PortDirection::Output));
};

string ConstantHDLDevice::GetInstanceName() { return inst_name; };

vector<HDLDevicePort *> &ConstantHDLDevice::GetPorts() { return ports; }

vector<string> ConstantHDLDevice::GetVHDLDeps() {
  return vector<string>{"ieee.std_logic_1164.all", "ieee.numeric_std.all"};
}

void ConstantHDLDevice::GenerateVHDLPrefix(ostream &vhdl) {}

void ConstantHDLDevice::GenerateVHDL(ostream &vhdl) {
  NumericPortType cType(value.bits.size(), value.is_signed);
  vhdl << "\t" << ports.at(0)->connectedNet->name << " <= "
       << ports.at(0)->type->VHDLCastFrom(
              &cType, string(value.is_signed ? "signed'(" : "unsigned'(") +
                          value.to_string() + ")")
       << ";" << endl;
}

void ConstantHDLDevice::AnnotateTiming(DeviceTiming *model) {
  ports.at(0)->connectedNet->timing_delay = HDLTimingValue<double>();
}

void ConstantHDLDevice::AnnotateLatency(DeviceTiming *model) {
  ports.at(0)->connectedNet->pipeline_latency = HDLTimingValue<int>();
}

ConstantHDLDevice::~ConstantHDLDevice() {
  for_each(ports.begin(), ports.end(), [](HDLDevicePort *p) { delete p; });
}

int ConstantHDLDevice::serial = 0;

BufferHDLDevice::BufferHDLDevice(HDLSignal *in, HDLSignal *out) {
  inst_name = "buf_" + to_string(serial++);
  ports.push_back(
      new HDLDevicePort("in", this, in->sigType, in, PortDirection::Input));
  ports.push_back(
      new HDLDevicePort("out", this, out->sigType, out, PortDirection::Output));
}

string BufferHDLDevice::GetInstanceName() { return inst_name; }

vector<HDLDevicePort *> &BufferHDLDevice::GetPorts() { return ports; }

vector<string> BufferHDLDevice::GetVHDLDeps() { return {}; }

void BufferHDLDevice::GenerateVHDLPrefix(ostream &vhdl) {}

void BufferHDLDevice::GenerateVHDL(ostream &vhdl) {
  vhdl << "\t" << ports.at(1)->connectedNet->name << " <= "
       << ports.at(1)->type->VHDLCastFrom(ports.at(0)->type,
                                          ports.at(0)->connectedNet->name)
       << ";" << endl;
}

void BufferHDLDevice::AnnotateTiming(DeviceTiming *model) {
  ports.at(1)->connectedNet->timing_delay =
      ports.at(0)->connectedNet->timing_delay;
}
void BufferHDLDevice::AnnotateLatency(DeviceTiming *model) {
  ports.at(1)->connectedNet->pipeline_latency =
      ports.at(0)->connectedNet->pipeline_latency;
}

BufferHDLDevice::~BufferHDLDevice() {
  for_each(ports.begin(), ports.end(), [](HDLDevicePort *p) { delete p; });
}

int BufferHDLDevice::serial = 0;

MultiplexerHDLDevice::MultiplexerHDLDevice(
    const vector<HDLSignal *> &mux_inputs, HDLSignal *mux_sel,
    HDLSignal *output) {
  inst_name = "mux_" + to_string(serial++);
  for (int i = 0; i < mux_inputs.size(); i++)
    ports.push_back(new HDLDevicePort("input" + to_string(i), this,
                                      mux_inputs.at(i)->sigType,
                                      mux_inputs.at(i), PortDirection::Input));
  size = mux_inputs.size();
  ports.push_back(new HDLDevicePort("select", this, mux_sel->sigType, mux_sel,
                                    PortDirection::Input));
  ports.push_back(new HDLDevicePort("output", this, output->sigType, output,
                                    PortDirection::Output));
}

string MultiplexerHDLDevice::GetInstanceName() { return inst_name; }

vector<HDLDevicePort *> &MultiplexerHDLDevice::GetPorts() { return ports; }

vector<string> MultiplexerHDLDevice::GetVHDLDeps() {
  return vector<string>{"ieee.std_logic_1164.all", "ieee.numeric_std.all"};
}

void MultiplexerHDLDevice::GenerateVHDLPrefix(ostream &vhdl) {}

void MultiplexerHDLDevice::GenerateVHDL(ostream &vhdl) {
  vhdl << "\t" << ports.back()->connectedNet->name << " <= ";
  for (int i = 0; i < size; i++) {
    if (i != 0)
      vhdl << "\t\t\t\t";
    vhdl << ports.back()->type->VHDLCastFrom(ports.at(i)->type,
                                             ports.at(i)->connectedNet->name);
    vhdl << " when unsigned(" << ports.at(ports.size() - 2)->connectedNet->name
         << ") == " << i << " else " << endl;
  }
  vhdl << "\t\t\t\t" <<  ports.back()->type->GetZero() << ";" << endl;
}

void MultiplexerHDLDevice::AnnotateTiming(DeviceTiming *model) {
  HDLTimingValue<double> inp_delay =
      (*max_element(ports.begin(), ports.end() - 1,
                    [](HDLDevicePort *a, HDLDevicePort *b) {
                      return a->connectedNet->timing_delay <
                             b->connectedNet->timing_delay;
                    }))
          ->connectedNet->timing_delay;
  // TODO: use model to calculate device delay
  double dev_delay = 0;
  ports.back()->connectedNet->timing_delay = inp_delay + dev_delay;
  // TODO: should we propogate clock domains?
}

void MultiplexerHDLDevice::AnnotateLatency(DeviceTiming *model) {
  HDLTimingValue<int> inp_latency =
      (*max_element(ports.begin(), ports.end() - 1,
                    [](HDLDevicePort *a, HDLDevicePort *b) {
                      return a->connectedNet->pipeline_latency <
                             b->connectedNet->pipeline_latency;
                    }))
          ->connectedNet->pipeline_latency;
  ports.back()->connectedNet->pipeline_latency = inp_latency;
}

MultiplexerHDLDevice::~MultiplexerHDLDevice() {
  for_each(ports.begin(), ports.end(), [](HDLDevicePort *p) { delete p; });
}

int MultiplexerHDLDevice::serial = 0;

} // namespace HDLGen
} // namespace RapidHLS
