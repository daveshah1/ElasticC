#pragma once
#include <fstream>
#include <string>
#include <vector>
using namespace std;
namespace RapidHLS {
enum MessageLevel { MSG_DEBUG = 0, MSG_NOTE, MSG_WARNING, MSG_ERROR };

enum ConsoleColour {
  COLOUR_BLACK = 0,
  COLOUR_RED,
  COLOUR_GREEN,
  COLOUR_YELLOW,
  COLOUR_BLUE,
  COLOUR_MAGENTA,
  COLOUR_CYAN,
  COLOUR_WHITE
};

void Console_SetForeColour(ConsoleColour clr);
void Console_ResetColour();
void Console_SetBright();
// Print a message to the console, if message level >= current verbosity level
// Error level messages will terminate execution
// Optionally takes a line number
void PrintMessage(MessageLevel level, string message, int line = -1);

// Print the startup banner
void PrintBanner(string appname);

// Find a file given one or more possible paths, searching all paths in
// the specified environment var and the current dir if specied
string FindFile(string filename, string envVar, bool includeCwd = false);
string FindFile(vector<string> filenames, string envVar,
                bool includeCwd = false);

// Return an execution-unique integer ID
int GetUniqueID();

namespace EnvironmentVars {
const string rhls_incdir = "RAPIDHLS_INCDR";
}

// Read an environment variable, adding extracted paths (expected to be
// seperated by a colon) to a vector
void ParseEnvironmentVar(string var, vector<string> &paths);

string GetVersion();

extern MessageLevel verbosity;
extern const string rhls_version;

// Return the number of bits needed to represent n values (used to determine
// address bus size)
int GetAddressBusSize(int length);
}
