#pragma once
#include <iostream>
#include <string>
#include <vector>
using namespace std;
namespace RapidHLS {
namespace HDLGen {
class HDLDevicePort;
class HDLSignal;
class HDLDevice {
public:
  virtual string GetInstanceName() = 0;
  virtual vector<HDLDevicePort *> &GetPorts() = 0;

  // Generate VHDL prefix (signals, constants, types etc)
  virtual void GenerateVHDLPrefix(ostream &vhdl);
  // Generate VHDL for the device itself
  virtual void GenerateVHDL(ostream &vhdl);
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

  void GenerateVHDLPrefix(ostream &vhdl);
  void GenerateVHDL(ostream &vhdl);

private:
  string inst_name;
};
};
};
