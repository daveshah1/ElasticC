#include <string>
#include <vector>
using namespace std;

namespace RapidHLS {
namespace HDLGen {
class HDLDevicePort;
// These constraints allow the resolution of delays and latencies throughout the
// design for timing analysis and pipelining purposes
class TimingConstraint {
public:
  // For output constraints, the clock [or input] to output delay value
  virtual bool HasDefinedDelayValue();
  virtual double GetOutputDelayValue();
  // For input constraints, the maximum allowed delay at the input
  virtual bool HasMaxDelayRequirement();
  virtual double GetMaxDelayValue();
  // Return constraints that the current constraint depends upon
  virtual vector<TimingConstraint *> GetDependencies();

  virtual bool IsConstraintMet();
};
// A fixed delay, based on some device parameter
class FixedDelayConstraint {
public:
  FixedDelayConstraint(string _param);

  bool HasDefinedDelayValue();
  double GetOutputDelayValue();

  string param;
};
// Delay for a generic device; calculated on the max input delay plus some fixed
// amount
class DeviceOutputDelayConstraint {
public:
  DeviceOutputDelayConstraint(const vector<TimingConstraint *> &_inputDelays,
                              FixedDelayConstraint *_deviceTpd);

  bool HasDefinedValue();
  double GetOutputDelayValue();
  vector<TimingConstraint *> GetDependencies();

  vector<TimingConstraint *> _inputDelays;
  FixedDelayConstraint *deviceTpd;
};
// This returns the delay of a given input port (which will itself has a timing
// constraint)
class PortDelayConstraint {
  PortDelayConstraint(HDLDevicePort *_port);

  // For output constraints, the clock [or input] to output delay value
  bool HasDefinedDelayValue();
  double GetOutputDelayValue();
  // For input constraints, the maximum allowed delay at the input
  bool HasMaxDelayRequirement();
  double GetMaxDelayValue();
  // Return constraints that the current constraint depends upon
  vector<TimingConstraint *> GetDependencies();

  bool IsConstraintMet();

  HDLDevicePort *port;
};
}
}
