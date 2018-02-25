#include "Phases.hpp"
#include "Util.hpp"
using namespace std;

namespace ElasticC {

ParserState LoadCode(string file) {
  ifstream ifs(file);
  if (!ifs)
    PrintMessage(MSG_ERROR, "failed to open input file ===" + file + "===");
  string str((std::istreambuf_iterator<char>(ifs)),
             std::istreambuf_iterator<char>());
	PrintMessage(MSG_DEBUG, "loaded input file " + file);
  return ParserState(str, file);
};

Parser::GlobalScope *DoParse(ParserState &code) {
  Parser::GlobalScope *gs = new Parser::GlobalScope();
	PrintMessage(MSG_DEBUG, "starting parse");
  Parser::ECCParser(code, *gs).ParseAll();
  return gs;
}

EvaluatedBlock EvaluateCode(Evaluator *eval, Parser::HardwareBlock *top) {
	PrintMessage(MSG_NOTE, "evaluating block ===" + top->name + "===");
  eval->EvaluateBlock(top);
  return eval->GetEvaluatedBlock();
}

void OptimiseBlock(EvaluatedBlock *block) {
	// TODO
}

SynthContext MakeHDLDesign(Parser::HardwareBlock *top, EvaluatedBlock *block) {
  return MakeSynthContext(top, block);
}

void OptimiseHDLDesign(HDLGen::HDLDesign *hdld, SynthContext &sc) {
  hdld->Prune();
}


void PipelineHDLDesign(HDLGen::HDLDesign *hdld, SynthContext &sc) {
  // TODO
}

void PrintTiming(HDLGen::HDLDesign *hdld, SynthContext &sc) {
  // TODO
}

void GenerateVHDL(HDLGen::HDLDesign *hdld, string file) {
  ofstream ofs(file);
  if (!ofs)
    PrintMessage(MSG_ERROR, "failed to open output file ===" + file + "===");
  hdld->GenerateVHDLFile(ofs);
}

}; // namespace ElasticC
