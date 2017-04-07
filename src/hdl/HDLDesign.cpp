#include "HDLDesign.hpp"
#include <algorithm>
using namespace std;

namespace RapidHLS {
namespace HDLGen {

HDLDesign::HDLDesign(string _name) : name(_name) {}

void HDLDesign::AddSignal(HDLSignal *sig) { signals.push_back(sig); }

HDLSignal *HDLDesign::CreateTempSignal(HDLPortType *type, string prefix) {
  static int inc = 0;
  HDLSignal *sig = new HDLSignal(prefix + "_" + to_string(inc++) + "_", type);
  AddSignal(sig);
  return sig;
}

void HDLDesign::AddDevice(HDLDevice *dev) { devices.push_back(dev); }
}
}
