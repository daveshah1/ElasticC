#include "DeviceTiming.hpp"

namespace RapidHLS {

// Default delay values for testing
double DeviceTiming::GetFFSetupTime() { return 0.1e-9; }
double DeviceTiming::GetFFPropogationDelay() { return 1e-9; }
double DeviceTiming::GetOperationDelay(OperationType ot,
                                       vector<int> operandWidths) {
  return 1e-9;
}

}; // namespace RapidHLS
