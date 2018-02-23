#include "ECCParser.hpp"
#include "Util.hpp"
#include "Evaluator.hpp"
#include "hdl/HDLDesign.hpp"
#include "hdl/HDLDevice.hpp"
#include "ParserCore.hpp"
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <boost/program_options.hpp>
using namespace std;
using namespace ElasticC;

using namespace boost::program_options;

int main(int argc, char const *argv[]) {
	PrintBanner("Elastic-C");

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


    variables_map vm;
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

	return 0;
}
