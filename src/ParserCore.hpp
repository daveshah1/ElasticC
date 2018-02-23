#pragma once
#include <string>
#include <vector>
using namespace std;
namespace ElasticC {
// This provides the core for the parser, a class that wraps around a string
// and allows token fetching, whitespace and comment skipping, etc
class ParserState {
public:
  ParserState(string _code);

  // Skip whitespace and comments
  void Skip();

  // Returns whether or not at end of code
  bool AtEnd();

  // Return the next char, without advancing the current position
  char PeekNext();
  // Return the next char, advancing the position by one
  char GetNext();
  // Same as above but return n characters
  string PeekNext(int n);
  string GetNext(int n);
  // Return true and consume a char if the next char matches a given char;
  // otherwise return false and don't consume char
  bool CheckMatchAndGet(char c);
  // Returns the next valid identifier or literal (or an empty string if the
  // next valid char does not meet the requirements)
  // If removeFromStream is true the position will be advanced accordingly
  string GetNextIdentOrLiteral(bool removeFromStream = true);
  // Neater way of specifying above with false
  string PeekNextIdentOrLiteral();

  // Find the next token given a list of token. Returns the index of the found
  // token if a token is found, or -1 otherwise
  // If removeFromStream is true the position will be advanced accordingly
  // RequireCompleteToken means that the token must be the entirety of the next
  // identifier
  // E.g. if 'for' is a valid token, then for( will match but foreach won't
  int FindToken(const vector<string> &tokens, bool removeFromStream = true,
                bool requireCompleteToken = false);

  // Return the current line number, for diangostic purposes
  int GetLine();

  string code;
  int pos = 0;
};
}
