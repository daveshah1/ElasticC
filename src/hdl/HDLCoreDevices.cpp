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
  // Multiplication keeps operand types to allow signed/unsigned and mixed
  // width multiplies, everything else casts to a common type
  for (int i = 0; i < ports.size() - 1; i++) {
    HDLPortType *type = ports.at(i)->type;
    if (oper == OperationType::B_MUL) {
      operands.at(i) = NumericPortType(type->GetWidth(), type->IsSigned())
                           .VHDLCastFrom(type, operands.at(i));
    } else {
      operands.at(i) =
          NumericPortType(width, is_signed).VHDLCastFrom(type, operands.at(i));
    }
  }

  switch (oper) {
  case B_ADD:
    value = operands.at(0) + " + " + operands.at(1);
    break;
  case B_SUB:
    value = operands.at(0) + " - " + operands.at(1);
    break;
  case B_MUL:
    value = operands.at(0) + " * " + operands.at(1);
    break;
  case B_BWAND:
    value = operands.at(0) + " and " + operands.at(1);
    break;
  case B_BWOR:
    value = operands.at(0) + " or " + operands.at(1);
    break;
  case B_BWXOR:
    value = operands.at(0) + " xor " + operands.at(1);
    break;
  // TODO: remaining operations
  default:
    throw runtime_error("operation type not supported in HDL");
  }

  NumericPortType resType(width, is_signed);
  vhdl << "\t" << ports.back()->connectedNet->name << " <= ";
  vhdl << ports.back()->connectedNet->sigType->VHDLCastFrom(&resType, value);
}

void OperationHDLDevice::AnnotateTiming(DeviceTiming *model) {
  double inp_delay =
      (*max_element(ports.begin(), ports.end() - 1, [](HDLDevicePort *a,
                                                       HDLDevicePort *b) {
        return a->connectedNet->timing_delay < b->connectedNet->timing_delay;
      }))->connectedNet->timing_delay;
  // TODO: use model to calculate device delay
  double dev_delay = 0;
  ports.back()->connectedNet->timing_delay = inp_delay + dev_delay;
  // TODO: should we propogate clock domains?
}

void OperationHDLDevice::AnnotateLatency(DeviceTiming *model) {
  int inp_latency =
      (*max_element(ports.begin(), ports.end() - 1, [](HDLDevicePort *a,
                                                       HDLDevicePort *b) {
        return a->connectedNet->pipeline_latency <
               b->connectedNet->pipeline_latency;
      }))->connectedNet->pipeline_latency;
  ports.back()->connectedNet->pipeline_latency = inp_latency;
}

OperationHDLDevice::~OperationHDLDevice() {
  for_each(ports.begin(), ports.end(), [](HDLDevicePort *p) { delete p; });
}

int OperationHDLDevice::serial = 0;
}
}
