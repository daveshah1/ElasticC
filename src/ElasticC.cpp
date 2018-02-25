#include "ECCParser.hpp"
#include "Util.hpp"
#include "Phases.hpp"

#include <iostream>
#include <cstdlib>
#include <boost/program_options.hpp>
using namespace std;
using namespace ElasticC;

using namespace boost::program_options;

int main(int argc, char const *argv[]) {
	exec_path = string(argv[0]);

	PrintBanner("Elastic-C");
	variables_map vm;
	try
  {
    options_description desc{"Allowed Options"};
    desc.add_options()
      ("help,h", "Display this message")
      ("verbose,v", "Increase output verbosity")
      ("quiet,q", "Decrease output verbosity")
			("output,o", value<string>(), "Specify output file")
			("input,i", value<string>(), "Specify input file");

		positional_options_description pdesc;
		pdesc.add("input", -1);


    store(command_line_parser(argc, argv).
          options(desc).positional(pdesc).run(), vm);
    notify(vm);

    if (vm.count("help") || !vm.count("input")) {
			cerr << "Usage: " << endl;
			cerr << "elasticc [options] input.ecc" << endl << endl;

      cerr << desc << endl;
			return 3;
    }
  }
  catch (const error &ex)
  {
    cerr << "Fatal errror: " << ex.what() << endl;
		cerr << "Use -h to see usage and available options" << endl;

		return 2;
  }

	if(vm.count("verbose"))
		verbosity = MSG_DEBUG;
	else if(vm.count("quiet"))
		verbosity = MSG_WARNING;

	ParserState ps = LoadCode(vm.at("input").as<string>());
	Parser::GlobalScope *gs;
	try {
		gs = DoParse(ps);
	} catch (Parser::parse_error &e) {
		PrintMessage(MSG_ERROR, "Parse Error: " + string(e.what()), ps.GetLine());
	}
	EvaluatedBlock evb;

	Parser::HardwareBlock *blktop = nullptr;
	if(gs->blocks.size() == 0) {
		PrintMessage(MSG_NOTE, "design contains no hardware blocks, nothing to do");
		return 0;
	} else if(gs->blocks.size() > 1 || vm.count("top")) {
		if(!vm.count("top")) {
			PrintMessage(MSG_ERROR, "multiple hardware blocks found but none specified, use --top to specify one");
		}
		for(auto blk : gs->blocks) {
			if(blk->name == vm.at("top").as<string>()) {
				blktop = blk;
				break;
			}
		}
		if(blktop == nullptr) {
			PrintMessage(MSG_ERROR, "hardware block ===" + vm.at("top").as<string>() + "=== was not found in design");
		}
	} else {
		blktop = gs->blocks.at(0);
	}

	try {
		evb = EvaluateCode(new SingleCycleEvaluator(gs), blktop);
	} catch (eval_error &e) {
		PrintMessage(MSG_ERROR, "Evaluation Error: " + string(e.what()));
	}

	OptimiseBlock(&evb);

	// Convert the optimised block to a HDL style netlist
	SynthContext sc = MakeHDLDesign(blktop, &evb);

	// Optimise the generated HDL design
	OptimiseHDLDesign(sc.design, sc);

	// Insert pipeline registers as needed in the HDL netlist
  PipelineHDLDesign(sc.design, sc);

	// Print a final timing and pipeling report
	PrintTiming(sc.design, sc);

	string outfile;
	if(vm.count("output"))
		outfile = vm.at("output").as<string>();
	else
		outfile = vm.at("input").as<string>() + ".vhd";

	// Save the HDL design to a VHDL file
	GenerateVHDL(sc.design, outfile);


	return 0;
}
