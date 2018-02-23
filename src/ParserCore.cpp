#include "ParserCore.hpp"
#include "Util.hpp"
namespace ElasticC {
ParserState::ParserState(string _code, string _filename) {
  code = _code;
  filename = _filename;
}

void ParserState::Skip() {
  bool inComment = false;
  bool inLineComment = false;
  while ((pos < (code.size())) &&
         (inComment || inLineComment ||
          (isspace(code[pos]) ||
           ((pos < (code.size() - 1)) && (code[pos] == '/') &&
            ((code[pos + 1] == '*') || (code[pos + 1] == '/')))))) {

    if ((!inComment) && (!inLineComment)) {
      if ((pos < (code.size() - 1)) && (code[pos] == '/') &&
          (code[pos + 1] == '*')) {
        pos++;
        inComment = true;
      } else if ((pos < (code.size() - 1)) && (code[pos] == '/') &&
                 (code[pos + 1] == '/')) {
        pos++;
        inLineComment = true;
      }
    } else if (inLineComment) {
      if (code[pos] == '\n') {
        inLineComment = false;
      }
    } else {
      if ((pos < (code.size() - 1)) && (code[pos] == '*') &&
          (code[pos + 1] == '/')) {
        inComment = false;
        pos++;
      }
    }
    pos++;
  }
}

bool ParserState::AtEnd() { return !(pos < code.size()); }

char ParserState::PeekNext() {
  if (AtEnd()) {
    PrintMessage(MSG_ERROR, "Unexpected end of code");
  }
  return code[pos];
}

char ParserState::GetNext() {
  if (AtEnd()) {
    PrintMessage(MSG_ERROR, "Unexpected end of code");
  }

  return code[pos++];
}

string ParserState::PeekNext(int n) {
  string str;
  for (int i = 0; i < n; i++) {
    str += PeekNext();
  }
  return str;
}

string ParserState::GetNext(int n) {
  string str;
  for (int i = 0; i < n; i++) {
    str += GetNext();
  }
  return str;
}

bool ParserState::CheckMatchAndGet(char c) {
  if (PeekNext() == c) {
    GetNext();
    return true;
  } else {
    return false;
  }
}

string ParserState::GetNextIdentOrLiteral(bool removeFromStream) {
  Skip();
  int intpos = pos;
  string temp = "";

  // Allow "-" followed by a digit
  if (((intpos + 1) < code.size())) {
    if ((code[intpos] == '-') && isdigit(code[intpos + 1])) {
      temp += code[intpos++];
      temp += code[intpos++];
    }
  }

  while ((intpos < code.size()) &&
         (isalnum(code[intpos]) || (code[intpos] == '_'))) {
    temp += code[intpos];
    intpos++;
  };

  if (removeFromStream)
    pos = intpos;
  return temp;
}

string ParserState::PeekNextIdentOrLiteral() {
  return GetNextIdentOrLiteral(false);
}

int ParserState::FindToken(const vector<string> &tokens, bool removeFromStream,
                           bool requireCompleteToken) {
  // We want to return the longest matching token, so some trickery is required
  int found_length = 0;
  int found_index = -1;

  if (AtEnd())
    return -1;

  for (int i = 0; i < tokens.size(); i++) {
    if (code.substr(pos, tokens[i].length()) == tokens[i]) {
      if (tokens[i].length() > found_length) {
        if (requireCompleteToken) {
          if ((tokens[i].length() + pos) != code.length()) {
            if (isalnum(code[tokens[i].length() + pos]) ||
                ((code[tokens[i].length() + pos]) == '_')) {
              continue;
            }
          }
        }
        found_index = i;
        found_length = tokens[i].length();
      }
    }
  }

  if (removeFromStream)
    pos += found_length;
  return found_index;
}

int ParserState::GetLine() {
  int line = 1;
  for (int i = 0; i < pos; i++) {
    if (code[i] == '\n') {
      line++;
    }
  }
  return line;
}
} // namespace ElasticC
