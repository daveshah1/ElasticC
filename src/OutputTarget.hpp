#pragma once
#include "EvalObject.hpp"
#include "EvaluatorState.hpp"
#include "ParserStructures.hpp"
#include <map>
#include <string>
#include <vector>
using namespace std;

namespace ElasticC {
class OutputTarget {
public:
  // General exposed interface
  virtual void AnalyseTimingAndPipeline(Parser::HardwareBlock *block);
  virtual string GenerateAllBlocks(vector<Parser::HardwareBlock> *blocks);

  // Target side interface

  // Whether or not target requires pipelining for timing purposes
  virtual bool RequiresPipelining() = 0;
  // Return output file extension
  virtual string GetFileExtension() = 0;

  // Code generation functions
  virtual string GenerateFileHeader() = 0;
  virtual string GenerateBlockHeader(Parser::HardwareBlock *block) = 0;
  virtual string GenerateInputStatement(Parser::HardwareBlock *block,
                                        Parser::Variable *input) = 0;
  virtual string GenerateOutputStatement(Parser::HardwareBlock *block,
                                         Parser::Variable *output) = 0;
  virtual string GenerateEvalObject(EvalObject *item);
  virtual string GenerateBlockFooter(Parser::HardwareBlock *block) = 0;
  virtual string GenerateFileFooter() = 0;

  // Timing analysis
  // Return propagation delay through an EvalObject
  virtual double GetTpd(EvalObject *item);
};
}
