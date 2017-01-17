#pragma once
#include <map>
#include <string>
#include <vector>
using namespace std;
namespace RapidHLS {
/*
This allows C++-style attributes to be specified,
in the form
[[attr]] or [[attr(value)]]
*/
class AttributeSet {
public:
  AttributeSet();
  void AddAttributeFromStr(string attrStr);
  bool HasAttribute(string key);
  string GetAttributeValue(string key);
  string GetAttributeValue(string key, string defaultValue);

private:
  map<string, string> attrs; // Map attribute name to value; or empty string if
                             // attribute without value
};
}
