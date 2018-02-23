#include "ECCParser.hpp"
#include "Util.hpp"
#include "Evaluator.hpp"
#include "hdl/HDLDesign.hpp"
#include "hdl/HDLDevice.hpp"
#include "ParserCore.hpp"
#include <iostream>
#include <cstdlib>
#include <boost/program_options.hpp>
using namespace std;
using namespace ElasticC;

using namespace boost::program_options;

ParserState LoadCode(string file) {
	ifstream ifs(file);
	string str((std::istreambuf_iterator<char>(ifs)),
	                 std::istreambuf_iterator<char>());
	return ParserState(str);
};

Parser::GlobalScope *DoParse(ParserState &code) {
	Parser::GlobalScope *gs = new Parser::GlobalScope();
	Parser::ECCParser(code, *gs).ParseAll();
	return gs;
}

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
	return 0;
}
