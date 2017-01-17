#include <string>
#include <vector>
using namespace std;
namespace RapidHLS {
namespace HDLGen {
class HDLDevicePort;
class HDLSignal;
class HDLDevice {
public:
  virtual string GetInstanceName() = 0;
  virtual vector<HDLDevicePort *> &GetPorts() = 0;
};
// Represents some arbitrary HDL device; for example a vendor provided primitive
// or user created VHDL component
class GenericHDLDevice : public HDLDevice {
public:
  GenericHDLDevice(string _deviceType, const vector<HDLDevicePort *> &_ports,
                   string _library = "");
  string deviceType;
  vector<HDLDevicePort *> ports;
  string library; // if a non-empty string this is the library containing the
                  // device; which will be included if necessary

  string GetInstanceName();
  vector<HDLDevicePort *> &GetPorts();

private:
  string inst_name;
};

// Represents a device created by the RapidHLS toolchain; the contents of which
// are specified as a netlist of other devices
class RapidUserDevice : public HDLDevice {
public:
  RapidUserDevice(string _deviceType, const vector<HDLDevicePort *> &_ports,
                  string _library = "");
  string deviceType;
  vector<HDLDevicePort *> ports;
  vector<HDLDevice *> devices;
  vector<HDLSignal *> signals;

  string GetInstanceName();
  vector<HDLDevicePort *> &GetPorts();

private:
  string inst_name;
};
};
};
