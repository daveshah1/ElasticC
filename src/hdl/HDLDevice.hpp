#pragma once
#include "timing/DeviceTiming.hpp"
#include <iostream>
#include <string>
#include <vector>
using namespace std;
namespace ElasticC {
namespace HDLGen {
class HDLDevicePort;
class HDLSignal;

class HDLDevice {
public:
  virtual string GetInstanceName() = 0;
  virtual vector<HDLDevicePort *> &GetPorts() = 0;

  // Return VHDL packages required by the device
  virtual vector<string> GetVHDLDeps();
  // Generate VHDL prefix (signals, constants, types etc)
  virtual void GenerateVHDLPrefix(ostream &vhdl);
  // Generate VHDL for the device itself
  virtual void GenerateVHDL(ostream &vhdl);

  // Annotate timing and latencies for this device only
  virtual void AnnotateTiming(DeviceTiming *model);
  virtual void AnnotateLatency(DeviceTiming *model);
};
// Represents some arbitrary HDL device; for example a vendor provided primitive
// or user created VHDL component
class GenericHDLDevice : public HDLDevice {
public:
  GenericHDLDevice(string _deviceType, const vector<HDLDevicePort *> &_ports,
                   string _library = "");
  string deviceType;
  vector<HDLDevicePort *> ports;
  string library; // if a non-empty string this is the library containing the
                  // device; which will be included if necessary

  string GetInstanceName();
  vector<HDLDevicePort *> &GetPorts();

  vector<string> GetVHDLDeps();
  void GenerateVHDLPrefix(ostream &vhdl);
  void GenerateVHDL(ostream &vhdl);

  void AnnotateTiming(DeviceTiming *model);
  void AnnotateLatency(DeviceTiming *model);

private:
  static int serial;
  string inst_name;
};
};
};
