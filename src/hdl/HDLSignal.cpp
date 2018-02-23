#include "HDLSignal.hpp"
#include <algorithm>
#include <cmath>
using namespace std;
namespace ElasticC {
namespace HDLGen {

double ClockInfo::GetRisingEdgePos() {
  return (fmod(phase + 180.0, 360) / 360.0) * (1.0 / frequency);
}
double ClockInfo::GetFallingEdgePos() {
  return (fmod(phase + 0.0, 360) / 360.0) * (1.0 / frequency);
}

HDLSignal::HDLSignal(string _name, HDLPortType *_type)
    : name(_name), sigType(_type) {}

void HDLSignal::ConnectToSignal(HDLSignal *other) {
  for_each(connectedPorts.begin(), connectedPorts.end(),
           [other](HDLDevicePort *p) { p->connectedNet = other; });
  other->connectedPorts.insert(other->connectedPorts.end(),
                               connectedPorts.begin(), connectedPorts.end());
  connectedPorts.clear();
}

void HDLSignal::ConnectToPort(HDLDevicePort *port) {
  if (port->connectedNet != nullptr) {
    remove(port->connectedNet->connectedPorts.begin(),
           port->connectedNet->connectedPorts.end(), port);
  }
  port->connectedNet = this;
  connectedPorts.push_back(port);
}

void HDLSignal::GenerateVHDL(ostream &vhdl) {
  vhdl << "\tsignal " << name << " : " << sigType->GetVHDLType() << ";" << endl;
}
}
}
