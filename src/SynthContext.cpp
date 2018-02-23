#include "SynthContext.hpp"
#include "Evaluator.hpp"
#include "ParserStatements.hpp"
#include "ParserStructures.hpp"
#include "hdl/HDLSignal.hpp"
#include "hdl/HDLDevicePort.hpp"
#include "hdl/HDLCoreDevices.hpp"
#include "hdl/HDLPortType.hpp"

#include <algorithm>
using namespace std;

namespace ElasticC {
using namespace HDLGen;
SynthContext MakeSynthContext(Parser::HardwareBlock *hwblk,
                              EvaluatedBlock *evb) {
  // Standard IO
  SynthContext ctx;
  ctx.design = new HDLDesign(hwblk->name);
  Parser::HardwareBlockParams &hp = hwblk->params;

  if(hp.has_clock) {
    ctx.clock = new HDLSignal("clock", new ClockSignalPortType());
    ctx.design->AddPortFromSig(ctx.clock, PortDirection::Input);
  } else {
    ctx.clock = ctx.design->gnd;
  }

  if(hp.has_cken) {
    ctx.clock_enable = new HDLSignal("clken", new LogicSignalPortType());
    ctx.design->AddPortFromSig(ctx.clock_enable, PortDirection::Input);
  } else {
    ctx.clock_enable = ctx.design->vcc;
  }

  if(hp.has_sync_rst) {
    ctx.reset = new HDLSignal("reset", new LogicSignalPortType());
    ctx.design->AddPortFromSig(ctx.reset, PortDirection::Input);
  } else {
    ctx.reset = ctx.design->gnd;
  }

  if(hp.has_den) {
    ctx.input_valid = new HDLSignal("input_valid", new LogicSignalPortType());
    ctx.design->AddPortFromSig(ctx.input_valid, PortDirection::Input);
  } else {
    ctx.input_valid = ctx.design->vcc;
  }

  ctx.output_valid = new HDLSignal("output_valid", new LogicSignalPortType());
  if(hp.has_den_out) {
    ctx.design->AddPortFromSig(ctx.output_valid, PortDirection::Output);
  }

  // Design IO
  // TODO :- generate wiring
  

  return ctx;
}

} // namespace ElasticC
