#pragma once
#include "hdl/HDLDesign.hpp"
#include "hdl/HDLSignal.hpp"
#include <map>
using namespace std;

namespace ElasticC {
class EvaluatorVariable;
// This is used to store clock signal info, etc, when synthesising EvalObjects
struct SynthContext {
  HDLGen::HDLDesign *design;
  HDLGen::HDLSignal *clock, *clock_enable, *data_enable, *reset;
  map<EvaluatorVariable *, HDLGen::HDLSignal *> varSignals;
};
};
