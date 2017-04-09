#include "HDLDevicePort.hpp"
#include "HDLSignal.hpp"
#include <algorithm>
using namespace std;
namespace RapidHLS {
namespace HDLGen {

HDLDevicePort::HDLDevicePort(string _name, HDLDevice *_dev, HDLPortType *_type,
                             HDLSignal *_net, PortDirection _dir)
    : name(_name), device(_dev), type(_type), connectedNet(_net), dir(_dir) {
  if (connectedNet != nullptr)
    connectedNet->ConnectToPort(this);
};

void HDLDevicePort::GenerateVHDL(ostream &vhdl, bool is_last) {
  vhdl << "\t\t" << name << " : "
       << ((dir == PortDirection::Input)
               ? "in"
               : ((dir == PortDirection::Output) ? "out" : "inout"))
       << " " << type->GetVHDLType() << (is_last ? "" : ";") << endl;
};

HDLDevicePort::~HDLDevicePort() {
  if (connectedNet != nullptr) {
    remove(connectedNet->connectedPorts.begin(),
           connectedNet->connectedPorts.end(), this);
  }
}
}
}