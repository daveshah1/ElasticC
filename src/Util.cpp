#include "Util.hpp"
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
namespace ElasticC {
/*
If you are using a terminal that does not support VT100 control codes, you will
need to
define BORING_MODE
*/

MessageLevel verbosity = MSG_NOTE;

string GetVersion() { return ecc_version; }

void Console_SetForeColour(ConsoleColour clr) {
#ifndef BORING_MODE
  cerr << "\033[" << (clr + 30) << "m";
#endif
}

void Console_ResetColour() {
#ifndef BORING_MODE
  cerr << "\033[0m";
#endif
}

void Console_SetBright() {
#ifndef BORING_MODE
  cerr << "\033[1m";
#endif
}

int GetUniqueID() {
  static int i;
  return i++;
}

void PrintMessage(MessageLevel level, string message, int line) {
  if (level >= verbosity) {
    ConsoleColour clr;
    switch (level) {
    case MSG_DEBUG:
      clr = COLOUR_BLUE;
      Console_SetForeColour(clr);
      cerr << "[DEBUG]";
      break;
    case MSG_NOTE:
      clr = COLOUR_GREEN;
      Console_SetForeColour(clr);
      cerr << "[NOTE ]";
      break;
    case MSG_WARNING:
      clr = COLOUR_YELLOW;
      Console_SetForeColour(clr);
      cerr << "[WARN ]";
      break;
    case MSG_ERROR:
      clr = COLOUR_RED;
      Console_SetForeColour(clr);
      cerr << "[ERROR]";
      break;
    }
    cerr << " ";
    if (line > 0) {
      cerr << "[" << setw(4) << line << "]";
    } else {
      cerr << "      ";
    }
    cerr << " ";

    bool is_bold = false;
    for (int i = 0; i < message.size(); i++) {
      if (message[i] == '\n') {
        // indent lines nicely
        cerr << endl << "               ";
      } else {
        if ((i < (message.size() - 2))) {
          if ((message[i] == '=') && (message[i + 1] == '=') &&
              (message[i + 2] == '=')) {
            is_bold = !is_bold;
            if (is_bold) {
              Console_SetBright();
            } else {
              Console_ResetColour();
              Console_SetForeColour(clr);
            }
            i += 2;
          } else {
            cerr << message[i];
          }

        } else {
          cerr << message[i];
        }
      }
    }
    cerr << endl;
  }
  Console_ResetColour();
  if (level == MSG_ERROR) {
    //  cerr << "compilation terminated due to error" << endl;
    exit(EXIT_FAILURE);
  }
}

void PrintBanner(string appname) {
  cout << appname << " version ";
  Console_SetForeColour(COLOUR_CYAN);
  cout << GetVersion() << endl;
  Console_ResetColour();
  cout << "Copyright (C) 2016-18 David Shah" << endl;
  cout << "This program is licensed under the MIT License and provided without "
          "warranty"
       << endl;
}

string FindFile(string filename, string envVar, bool includeCwd) {
  return FindFile(vector<string>{filename}, envVar, includeCwd);
}
string FindFile(vector<string> filenames, string envVar, bool includeCwd) {
  namespace fs = boost::filesystem;
  vector<fs::path> basePaths;
  if (includeCwd) {
    basePaths.push_back(fs::current_path());
  }
  vector<string> includeDirStrs;
  ParseEnvironmentVar(envVar, includeDirStrs);
  for (auto incdir : includeDirStrs) {
    basePaths.push_back(fs::path(incdir));
  }
  for (auto base : basePaths) {
    for (auto file : filenames) {
      if (fs::exists(base / file)) {
        return (base / file).string();
      }
    }
  }
  return "";
}

void ParseEnvironmentVar(string var, vector<string> &paths) {
  char *environ = getenv(var.c_str());
  if (environ != nullptr) {
    string envs(environ);
    int sepPos = -1;
    do {
      // Look for colon seperated values
      string dir = "";
      bool quotedDir = false;
      // But first check for a string wrapped in quotation marks
      if (envs[0] == '"') {
        envs = envs.substr(1);
        int endquote = envs.find("\"");
        if (endquote == string::npos) {
          PrintMessage(MSG_ERROR, "error processing environment");
        }
        dir = envs.substr(0, endquote);
        envs = envs.substr(endquote + 1);
        quotedDir = true;
      }

      sepPos = envs.find(":");
      if (sepPos == string::npos) {
        // no seperator, look at whole string
        if (!quotedDir)
          dir = envs;
      } else {
        if (!quotedDir)
          dir = envs.substr(0, sepPos);
        envs = envs.substr(sepPos + 1);
      }
      if (dir != "") {
        paths.push_back(dir);
      }
    } while (sepPos != string::npos);
  }
}

int GetAddressBusSize(int length) {
  int l = 2;
  for (int i = 1; i < 32; i++) {
    if (length <= l)
      return i;
    l *= 2;
  }
  return 32;
}
}
