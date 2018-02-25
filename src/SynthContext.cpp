#include "SynthContext.hpp"
#include "Evaluator.hpp"
#include "ParserStatements.hpp"
#include "ParserStructures.hpp"
#include "hdl/HDLCoreDevices.hpp"
#include "hdl/HDLDevicePort.hpp"
#include "hdl/HDLPortType.hpp"
#include "hdl/HDLSignal.hpp"

#include <algorithm>
using namespace std;

namespace ElasticC {
using namespace HDLGen;

static void UnpackInput(HDLSignal *inpsig, SynthContext &sc,
                        EvaluatorVariable *ev) {
  if (ev->IsScalar()) {
    string hdlname = "ecc_int_" + ev->name;
    HDLPortType *pty = ev->GetType()->GetHDLType();
    HDLSignal *sig = new HDLSignal(hdlname, pty);
    sc.varSignals[ev] = sig;
    sc.drivenSignals.insert(ev);
    sc.design->AddSignal(sig);
    int offset_low = ev->GetBitOffset();
    int offset_high = (pty->GetWidth() - 1) + offset_low;
    sc.design->AddDevice(
        new BufferHDLDevice(inpsig, sig, HDLBitSlice(offset_high, offset_low)));
  } else {
    vector<EvaluatorVariable *> chld = ev->GetAllChildren();
    for_each(chld.begin(), chld.end(),
             [&](EvaluatorVariable *ch) { UnpackInput(inpsig, sc, ch); });
    // Ignore during evaluation
    sc.varSignals[ev] = inpsig;
    sc.drivenSignals.insert(ev);
  }
}

static void PackOutput(vector<pair<HDLSignal *, HDLBitSlice>> &outputSlices,
                       SynthContext &sc, EvaluatorVariable *ev) {
  if (ev->IsScalar()) {
    string hdlname = "ecc_int_" + ev->name;
    HDLPortType *pty = ev->GetType()->GetHDLType();
    HDLSignal *sig = new HDLSignal(hdlname, pty);
    sc.varSignals[ev] = sig;
    sc.design->AddSignal(sig);
    int offset_low = ev->GetBitOffset();
    int offset_high = (pty->GetWidth() - 1) + offset_low;
    outputSlices.push_back(
        make_pair(sig, HDLBitSlice(offset_high, offset_low)));
  } else {
    vector<EvaluatorVariable *> chld = ev->GetAllChildren();
    for_each(chld.begin(), chld.end(),
             [&](EvaluatorVariable *ch) { PackOutput(outputSlices, sc, ch); });
    // Ignore during evaluation
    sc.varSignals[ev] = nullptr;
    sc.drivenSignals.insert(ev);
  }
}

static void GenerateIO(SynthContext &sc, EvaluatorVariable *ev, bool is_input,
                       bool is_output) {
  HDLPortType *topType =
      (ev->GetType()->GetWidth() == 1)
          ? static_cast<HDLPortType *>(new LogicSignalPortType())
          : static_cast<HDLPortType *>(
                new LogicVectorPortType(ev->GetType()->GetWidth()));
  HDLSignal *iosig = new HDLSignal(ev->name, topType);
  if (is_input) {
    sc.design->AddPortFromSig(iosig, PortDirection::Input);
    ev->SetBitOffset(0);
    UnpackInput(iosig, sc, ev);
  } else if (is_output) {
    sc.design->AddPortFromSig(iosig, PortDirection::Output);
    ev->SetBitOffset(0);
    vector<pair<HDLSignal *, HDLBitSlice>> outputSlices;
    PackOutput(outputSlices, sc, ev);
    sc.design->AddDevice(new CombinerHDLDevice(outputSlices, iosig));
  }
}

SynthContext MakeSynthContext(Parser::HardwareBlock *hwblk,
                              EvaluatedBlock *evb) {
  // Standard IO
  SynthContext ctx;
  ctx.design = new HDLDesign(hwblk->name);
  Parser::HardwareBlockParams &hp = hwblk->params;

  if (hp.has_clock) {
    ctx.clock = new HDLSignal("clock", new ClockSignalPortType());
    ctx.design->AddPortFromSig(ctx.clock, PortDirection::Input);
  } else {
    ctx.clock = ctx.design->gnd;
  }

  if (hp.has_cken) {
    ctx.clock_enable = new HDLSignal("clken", new LogicSignalPortType());
    ctx.design->AddPortFromSig(ctx.clock_enable, PortDirection::Input);
  } else {
    ctx.clock_enable = ctx.design->vcc;
  }

  if (hp.has_sync_rst) {
    ctx.reset = new HDLSignal("reset", new LogicSignalPortType());
    ctx.design->AddPortFromSig(ctx.reset, PortDirection::Input);
  } else {
    ctx.reset = ctx.design->gnd;
  }

  if (hp.has_den) {
    ctx.input_valid = new HDLSignal("input_valid", new LogicSignalPortType());
    ctx.design->AddPortFromSig(ctx.input_valid, PortDirection::Input);
  } else {
    ctx.input_valid = ctx.design->vcc;
  }

  ctx.output_valid = new HDLSignal("output_valid", new LogicSignalPortType());
  if (hp.has_den_out) {
    ctx.design->AddPortFromSig(ctx.output_valid, PortDirection::Output);
  }

  // Design IO
  for (auto inp : hwblk->inputs)
    GenerateIO(ctx, evb->parserVariables.at(inp), true, false);
  for (auto op : hwblk->outputs)
    GenerateIO(ctx, evb->parserVariables.at(op), false, true);

  // Internal signals
  for (auto sigval : evb->vars) {
    if (ctx.varSignals.find(sigval.first) == ctx.varSignals.end()) {
      HDLSignal *hdlsig = new HDLSignal(sigval.first->name,
                                        sigval.first->GetType()->GetHDLType());
      ctx.design->AddSignal(hdlsig);
      ctx.varSignals[sigval.first] = hdlsig;
    }
  }
  for (auto sigval : evb->vars) {
    if (ctx.drivenSignals.find(sigval.first) == ctx.drivenSignals.end()) {
      ctx.drivenSignals.insert(sigval.first);
      sigval.second->Synthesise(evb->eval, ctx,
                                ctx.varSignals.at(sigval.first));
    }
  }

  return ctx;
}

} // namespace ElasticC
