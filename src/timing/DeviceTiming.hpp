#pragma once
#include "Operations.hpp"
// Provides a generic interface for devices to calculate timings
namespace RapidHLS {
class DeviceTiming {
public:
  double GetFFSetupTime();
  double GetFFPropogationDelay();
  double GetOperationDelay(OperationType ot, vector<int> operandWidths);
};
};
