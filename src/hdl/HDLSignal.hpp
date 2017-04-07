#pragma once
#include "HDLDevice.hpp"
#include "HDLDevicePort.hpp"
#include "HDLPortType.hpp"
#include <iostream>
#include <string>
#include <vector>
using namespace std;

namespace RapidHLS {
namespace HDLGen {
struct ClockInfo {
public:
  double frequency = 50e6; // frequency in Hz
  double phase = 0;        // phase in degrees
  double duty = 0.5;       // duty cycle as a fraction

  // Get position of first rising edge in seconds
  double GetRisingEdgePos();
  double GetFallingEdgePos();
};

class HDLSignal {
public:
  string name;
  HDLPortType *sigType;
  vector<HDLDevicePort *> connectedPorts;
  ClockInfo clockInfo; // clock type signals only

  // Connect another signal to this one, replacing all instances of this signal
  // with the passed one
  void ConnectToSignal(HDLSignal *other);
  // Connect to a device port
  void ConnectToPort(HDLDevicePort *port);
  // Generate a VHDL signal definition
  void GenerateVHDL(ostream &vhdl);
};
}
}
