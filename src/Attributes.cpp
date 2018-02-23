#include "Attributes.hpp"
#include <algorithm>
#include <iterator>
#include <stdexcept>
using namespace std;

namespace ElasticC {

AttributeSet::AttributeSet(){};

void AttributeSet::AddAttributeFromStr(string attrStr) {
  string attrName, attrValue;
  if (attrStr.find('(') != string::npos) {
    attrName = attrStr.substr(0, attrStr.find('('));
    attrValue = attrStr.substr(attrStr.find('(') + 1);
    if (attrValue.back() != ')')
      throw runtime_error("invalid attribute specifier");
    attrValue = attrValue.substr(0, attrValue.length() - 1);
  } else {
    attrName = attrStr;
    attrValue = "";
  }
  attrs[attrName] = attrValue;
};

bool AttributeSet::HasAttribute(string key) {
  return (attrs.find(key) != attrs.end());
}

string AttributeSet::GetAttributeValue(string key) { return attrs.at(key); }

string AttributeSet::GetAttributeValue(string key, string defaultValue) {
  if (HasAttribute(key)) {
    return GetAttributeValue(key);
  } else {
    return defaultValue;
  }
}
}
