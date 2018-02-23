#include "Phases.hpp"
#include "Util.hpp"
using namespace std;

namespace ElasticC {

ParserState LoadCode(string file) {
  ifstream ifs(file);
  if (!ifs)
    PrintMessage(MSG_ERROR, "failed to open input file " + file);
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
  eval->EvaluateBlock(top);
  return eval->GetEvaluatedBlock();
}

}; // namespace ElasticC
