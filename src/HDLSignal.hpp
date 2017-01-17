#include "HDLDevice.hpp"
#include "HDLDevicePort.hpp"
#include "HDLPortType.hpp"
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
};
}
}
