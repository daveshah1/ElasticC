#pragma once
#include "HDLDevice.hpp"
#include "HDLSignal.hpp"
#include "HDLDevicePort.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <list>

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
  void AddPort(HDLDevicePort *port);
  void AddPortFromSig(HDLSignal *sig, PortDirection dir);

  HDLSignal *CreateTempSignal(HDLPortType *type, string prefix = "temp");
  void AddDevice(HDLDevice *dev);

  void RemoveDevice(HDLDevice *dev);
  void RemoveSignal(HDLSignal *sig);
  // Return the number of input ports driven by a signal
  int GetSignalFanout(HDLSignal *sig);
  // Return the total number of inputs drvien by all outputs of a device
  int GetDeviceFanout(HDLDevice *dev);

  // Remove devices and signals that have no bearing on the output
  void Prune();
  // Run a single pass removing devices with no fanout, returning true on change
  bool PruneDevicesPass();
  // Run a single pass pruning nets with no connections
  void PruneNetsPass();

  void GenerateVHDLFile(ostream &out);

  // Special constant forced signals
  HDLSignal *gnd, *vcc;
};
}
}
