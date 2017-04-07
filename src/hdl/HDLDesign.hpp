#pragma once
#include "HDLDevice.hpp"
#include "HDLSignal.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace RapidHLS {
namespace HDLGen {

// Represents the top level of an entity
class HDLDesign {
public:
  HDLDesign(string _name);

  string name;
  vector<HDLSignal *> signals;
  vector<HDLPort *> ports;
  vector<HDLDevice *> devices;

  void AddSignal(HDLSignal *sig);
  HDLSignal *CreateTempSignal(HDLPortType *type, string prefix = "");
  void AddDevice(HDLDevice *dev);

  void GenerateVHDLFile(ostream &out);
};
}
}
