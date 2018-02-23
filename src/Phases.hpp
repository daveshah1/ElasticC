#pragma once
#include "ECCParser.hpp"
#include "EvalObject.hpp"
#include "Evaluator.hpp"
#include "SynthContext.hpp"
#include "hdl/HDLDesign.hpp"

using namespace std;

namespace ElasticC {
// Load code from an input file into a ParserState
ParserState LoadCode(string file);

// Parse imported code and return the resulting GlobalScope
Parser::GlobalScope *DoParse(ParserState &code);

// Evaluate the parsed code using a given Evaluator and return the evalued
// block
EvaluatedBlock EvaluateCode(Evaluator *eval, Parser::HardwareBlock *top);

// Optimise the evaluated block
void OptimiseBlock(EvaluatedBlock *block);

// Convert the optimised block to a HDL style netlist
SynthContext MakeHDLDesign(Parser::HardwareBlock *top, EvaluatedBlock *block);

// Insert pipeline registers as needed in the HDL netlist
void PipelineHDLDesign(HDLGen::HDLDesign *hdld, SynthContext &sc);

// Print a final timing and pipeling report
void PrintTiming(HDLGen::HDLDesign *hdld, SynthContext &sc);

// Save the HDL design to a VHDL file
void GenerateVHDL(HDLGen::HDLDesign *hdld, string file);

} // namespace ElasticC
