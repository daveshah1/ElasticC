#pragma once
#include "BitConstant.hpp"
#include "HDLDevice.hpp"
#include "Operations.hpp"
#include "HDLSignal.hpp"
#include <optional>
using namespace std;

namespace ElasticC {
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
  string inst_name;
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
// typecast as needed. It can also select a bitslice of the input signal
class BufferHDLDevice : public HDLDevice {
public:
  BufferHDLDevice(HDLSignal *in, HDLSignal *out, optional<HDLBitSlice> _slice = optional<HDLBitSlice>());
  string GetInstanceName();
  vector<HDLDevicePort *> &GetPorts();

  vector<string> GetVHDLDeps();
  void GenerateVHDLPrefix(ostream &vhdl);
  void GenerateVHDL(ostream &vhdl);

  void AnnotateTiming(DeviceTiming *model);
  void AnnotateLatency(DeviceTiming *model);

  ~BufferHDLDevice();

private:
  optional<HDLBitSlice> slice;
  static int serial;
  string inst_name;
  vector<HDLDevicePort *> ports;
};

// A device which combines multiple slices into a single signal
class CombinerHDLDevice : public HDLDevice {
public:
  CombinerHDLDevice(const vector<pair<HDLSignal *, HDLBitSlice>> &inputs, HDLSignal *output);
  string GetInstanceName();
  vector<HDLDevicePort *> &GetPorts();

  vector<string> GetVHDLDeps();
  void GenerateVHDLPrefix(ostream &vhdl);
  void GenerateVHDL(ostream &vhdl);

  void AnnotateTiming(DeviceTiming *model);
  void AnnotateLatency(DeviceTiming *model);

  ~CombinerHDLDevice();

private:
  vector<pair<HDLSignal *, HDLBitSlice>> input_slices;
  static int serial;
  string inst_name;
  vector<HDLDevicePort *> ports;
};

};
};
