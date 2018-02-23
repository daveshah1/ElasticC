#include "HDLDevice.hpp"
#include <algorithm>
using namespace std;
namespace ElasticC {
namespace HDLGen {

vector<string> HDLDevice::GetVHDLDeps() { return vector<string>{}; };
void HDLDevice::GenerateVHDLPrefix(ostream &vhdl) {}
void HDLDevice::GenerateVHDL(ostream &vhdl) {}
void HDLDevice::AnnotateTiming(DeviceTiming *model) {}
void HDLDevice::AnnotateLatency(DeviceTiming *model) {}
}
}
