#pragma once
#include "BitConstant.hpp"
#include "HDLDevice.hpp"
#include "Operations.hpp"
using namespace std;

namespace RapidHLS {
namespace HDLGen {
// A device which performs an operation such as arithmetic, comparison or logic
class OperationHDLDevice : public HDLDevice {
public:
  OperationHDLDevice(OperationType _oper, const vector<HDLSignal *> &inputs,
                     HDLSignal *output);
  string GetInstanceName();
  vector<HDLDevicePort *> &GetPorts();

  vector<string> GetVHDLDeps();
  void GenerateVHDLPrefix(ostream &vhdl);
  void GenerateVHDL(ostream &vhdl);

  void AnnotateTiming(DeviceTiming *model);
  void AnnotateLatency(DeviceTiming *model);

  ~OperationHDLDevice();

private:
  static int serial;
  string inst_name;
  OperationType oper;
  vector<HDLDevicePort *> ports;
};

// A basic register, either for a functional delay or for pipelining
class RegisterHDLDevice : public HDLDevice {
public:
  RegisterHDLDevice(HDLSignal *d, HDLSignal *clk, HDLSignal *q, HDLSignal *en,
                    HDLSignal *rst, bool _is_pipeline = false);
  string GetInstanceName();
  vector<HDLDevicePort *> &GetPorts();

  vector<string> GetVHDLDeps();
  void GenerateVHDLPrefix(ostream &vhdl);
  void GenerateVHDL(ostream &vhdl);

  void AnnotateTiming(DeviceTiming *model);
  void AnnotateLatency(DeviceTiming *model);

  ~RegisterHDLDevice();

private:
  static int serial;
  string inst_name;
  bool is_pipeline;
  vector<HDLDevicePort *> ports;
};

// A multiplexer, used for conditionals and array access
class MultiplexerHDLDevice : public HDLDevice {
public:
  MultiplexerHDLDevice(const vector<HDLSignal *> &mux_inputs,
                       HDLSignal *mux_sel, HDLSignal *output);
  string GetInstanceName();
  vector<HDLDevicePort *> &GetPorts();

  vector<string> GetVHDLDeps();
  void GenerateVHDLPrefix(ostream &vhdl);
  void GenerateVHDL(ostream &vhdl);

  void AnnotateTiming(DeviceTiming *model);
  void AnnotateLatency(DeviceTiming *model);

  ~MultiplexerHDLDevice();

private:
  static int serial;
  int size = 2;
  vector<HDLDevicePort *> ports;
};

// A device which outputs a constant
class ConstantHDLDevice : public HDLDevice {
public:
  ConstantHDLDevice(BitConstant _value, HDLSignal *output);
  string GetInstanceName();
  vector<HDLDevicePort *> &GetPorts();

  vector<string> GetVHDLDeps();
  void GenerateVHDLPrefix(ostream &vhdl);
  void GenerateVHDL(ostream &vhdl);

  void AnnotateTiming(DeviceTiming *model);
  void AnnotateLatency(DeviceTiming *model);

  ~ConstantHDLDevice();

private:
  static int serial;
  BitConstant value;
  string inst_name;
  vector<HDLDevicePort *> ports;
};

// A buffer which passes through values without delay but will automatically
// typecast as needed
class BufferHDLDevice : public HDLDevice {
public:
  BufferHDLDevice(HDLSignal *in, HDLSignal *out);
  string GetInstanceName();
  vector<HDLDevicePort *> &GetPorts();

  vector<string> GetVHDLDeps();
  void GenerateVHDLPrefix(ostream &vhdl);
  void GenerateVHDL(ostream &vhdl);

  void AnnotateTiming(DeviceTiming *model);
  void AnnotateLatency(DeviceTiming *model);

  ~BufferHDLDevice();

private:
  static int serial;
  string inst_name;
  vector<HDLDevicePort *> ports;
};
};
};
