#pragma once
#include "HDLDevice.hpp"
#include "HDLSignal.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace ElasticC {
namespace HDLGen {

// Represents the top level of an entity
class HDLDesign {
public:
  HDLDesign(string _name);

  string name;
  vector<HDLSignal *> signals;
  vector<HDLDevicePort *> ports;
  vector<HDLDevice *> devices;

  void AddSignal(HDLSignal *sig);
  HDLSignal *CreateTempSignal(HDLPortType *type, string prefix = "temp");
  void AddDevice(HDLDevice *dev);

  void GenerateVHDLFile(ostream &out);

  // Special constant forced signals
  HDLSignal *gnd, *vcc;
};
}
}
