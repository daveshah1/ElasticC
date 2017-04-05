#pragma once
#include "HDLPortType.hpp"
#include <string>
#include <vector>
using namespace std;

namespace RapidHLS {
namespace HDLGen {
class HDLSignal;
class HDLDevice;

enum class PortDirection { Input, Output, Bidir };

class HDLPort {
public:
  string name;
  HDLDevice *device;
  HDLPortType *type;
  HDLSignal *connectedNet;
  PortDirection dir;
};
}
}
