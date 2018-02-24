#pragma once
#include "hdl/HDLDesign.hpp"
#include "hdl/HDLSignal.hpp"
#include <map>
#include <set>
using namespace std;

namespace ElasticC {
class EvaluatorVariable;
struct EvaluatedBlock;
namespace Parser {
class HardwareBlock;
}
// This is used to store clock signal info, etc, when synthesising EvalObjects
struct SynthContext {
  HDLGen::HDLDesign *design;
  HDLGen::HDLSignal *clock, *clock_enable, *input_valid, *output_valid, *reset;
  map<EvaluatorVariable *, HDLGen::HDLSignal *> varSignals;
  set<EvaluatorVariable *> drivenSignals;
};

// Construct a SynthContext and make a skeleton HDL design from a hardware block
// and the evaluator output
SynthContext MakeSynthContext(Parser::HardwareBlock *hwblk,
                              EvaluatedBlock *evb);

}; // namespace ElasticC
