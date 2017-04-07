#include "HDLDevicePort.hpp"
#include <algorithm>
using namespace std;
namespace RapidHLS {
namespace HDLGen {
void HDLDevicePort::GenerateVHDL(ostream &vhdl, bool is_last) {
  vhdl << "\t\t" << name << " : "
       << ((dir == PortDirection::Input)
               ? "in"
               : ((dir == PortDirection::Output) ? "out" : "inout"))
       << " " << type->GetVHDLType() << (is_last ? "" : ";") << endl;
};
}
}
