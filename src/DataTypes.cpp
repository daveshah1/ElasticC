#include "DataTypes.hpp"
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;

namespace RapidHLS {

DataType *DataType::GetBaseType() {
  throw runtime_error(GetName() + " has no base type");
}

DataType *DataType::GetMemberType(string member) {
  throw runtime_error(GetName() + " has no member named " + member);
}

HDLGen::HDLPortType *DataType::GetHDLType() {
  return new HDLGen::LogicVectorPortType(GetWidth());
}

IntegerType::IntegerType(int _width, bool _is_signed) {
  width = _width;
  is_signed = _is_signed;
}

string IntegerType::GetName() {
  if (is_auto) {
    return "auto";
  } else if (is_signed) {
    return "signed<" + to_string(width) + ">";
  } else {
    return "unsigned<" + to_string(width) + ">";
  }
}

int IntegerType::GetWidth() { return width; }

vector<int> IntegerType::GetDimensions() { return vector<int>{}; }

bool IntegerType::Equals(DataType *other) {
  IntegerType *it = dynamic_cast<IntegerType *>(other);
  if (it == nullptr) {
    return false;
  } else {
    return ((it->width) == width) && ((it->is_signed) == is_signed);
  }
}

HDLGen::HDLPortType *IntegerType::GetHDLType() {
  return new HDLGen::NumericPortType(width, is_signed);
}

ArrayType::ArrayType(DataType *_baseType, int _length) {
  baseType = _baseType;
  length = _length;
}

string ArrayType::GetName() {
  return baseType->GetName() + "[" + to_string(length) + "]";
}

int ArrayType::GetWidth() { return length * baseType->GetWidth(); }

vector<int> ArrayType::GetDimensions() { return vector<int>{length}; }

bool ArrayType::Equals(DataType *other) {
  ArrayType *art = dynamic_cast<ArrayType *>(other);
  if (art == nullptr) {
    return false;
  } else {
    return (baseType->Equals(art->baseType) && (art->length == length));
  }
}

DataType *ArrayType::GetBaseType() { return baseType; };

HDLGen::HDLPortType *ArrayType::GetHDLType() {
  return new HDLGen::LogicVectorPortType(GetWidth());
}

StreamType::StreamType(DataType *_baseType, bool _2d, int _length, int _height,
                       int _lineWidth)
    : baseType(_baseType), isStream2d(_2d), length(_length), height(_height),
      lineWidth(_lineWidth) {}

string StreamType::GetName() {
  if (isStream2d) {
    return "stream2d<" + baseType->GetName() + ", " + to_string(length) + ", " +
           to_string(height) + ", " + to_string(lineWidth) + ">";

  } else {
    return "stream<" + baseType->GetName() + ", " + to_string(length) + ">";
  }
}

int StreamType::GetWidth() {
  return baseType->GetWidth(); // the nature of a stream means its port width is
                               // only one unit
}

vector<int> StreamType::GetDimensions() {
  if (isStream2d) {
    return vector<int>{length, height};
  } else {
    return vector<int>{length};
  }
};

bool StreamType::Equals(DataType *other) {
  StreamType *st = dynamic_cast<StreamType *>(other);
  if (st == nullptr) {
    return false;
  } else if (isStream2d) {
    return ((st->isStream2d) && (baseType->Equals(st->baseType)) &&
            (st->length == length) && (st->height == height) &&
            (st->lineWidth == lineWidth));
  } else {
    return ((!st->isStream2d) && (baseType->Equals(st->baseType)) &&
            (st->length == length));
  }
}

DataType *StreamType::GetBaseType() { return baseType; };

HDLGen::HDLPortType *StreamType::GetHDLType() {
  throw runtime_error("stream type has no HDL equivalent");
}

RAMType::RAMType(IntegerType _baseType, int _length)
    : baseType(_baseType), length(_length) {}

string RAMType::GetName() {
  if (is_rom) {
    return "rom<" + baseType.GetName() + ", " + to_string(length) + ">";
  } else {
    return "ram<" + baseType.GetName() + ", " + to_string(length) + ">";
  }
}

int RAMType::GetWidth() { return baseType.GetWidth(); }

vector<int> RAMType::GetDimensions() { return vector<int>{length}; }

bool RAMType::Equals(DataType *other) {
  RAMType *rt = dynamic_cast<RAMType *>(other);
  if (rt == nullptr) {
    return false;
  } else {
    return ((baseType.Equals(&(rt->baseType))) && (rt->length == length) &&
            (rt->is_rom == is_rom));
  }
}

DataType *RAMType::GetBaseType() { return &baseType; };

HDLGen::HDLPortType *RAMType::GetHDLType() {
  throw runtime_error("RAM type has no HDL equivalent");
}

bool operator==(const DataStructureItem &a, const DataStructureItem &b) {
  return (a.name == b.name) && (a.type->Equals(b.type));
}

DataStructureItem::DataStructureItem() {}
DataStructureItem::DataStructureItem(string _name, DataType *_type)
    : name(_name), type(_type){};

StructureType::StructureType() {}

StructureType::StructureType(string _name,
                             const vector<DataStructureItem> &_content)
    : structName(_name), content(_content){};

string StructureType::GetName() { return structName; }

int StructureType::GetWidth() {
  int width = 0;
  for (auto child : content) {
    width += child.type->GetWidth();
  }
  return width;
}
vector<int> StructureType::GetDimensions() { return vector<int>{}; }

bool StructureType::Equals(DataType *other) {
  StructureType *st = dynamic_cast<StructureType *>(other);
  if (st == nullptr) {
    return false;
  } else {
    return st->content == content;
  }
};

DataType *StructureType::GetMemberType(string member) {
  auto m = find_if(
      content.begin(), content.end(),
      [member](const DataStructureItem &itm) { return itm.name == member; });
  if (m == content.end()) {
    throw runtime_error("structure " + structName +
                        " contains no member named " + member);
  } else {
    return m->type;
  }
}

HDLGen::HDLPortType *StructureType::GetHDLType() {
  return new HDLGen::LogicVectorPortType(GetWidth());
}
}
