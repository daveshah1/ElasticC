
#pragma once
#include "hdl/HDLPortType.hpp"
#include <string>
#include <vector>
using namespace std;
namespace RapidHLS {

/*
The generic interface for definable data types
*/
class DataType {
public:
  virtual string GetName() = 0; // return user-friendly name
  virtual int GetWidth() = 0;   // its width in bits

  // Return a list of array dimension sizes, or the empty list if scalar
  virtual vector<int> GetDimensions() = 0;

  virtual bool Equals(DataType *other) = 0;

  virtual DataType *GetBaseType();
  virtual DataType *GetMemberType(string member);

  virtual HDLGen::HDLPortType *GetHDLType();
};

/*
A synthesizable fixed width signed or unsigned integer
*/
class IntegerType : public DataType {
public:
  IntegerType(int _width, bool _is_signed);

  int width;
  bool is_signed;
  vector<int> GetDimensions();
  string GetName();
  int GetWidth();
  bool Equals(DataType *other);
  // If true, is implicit type
  bool is_auto = false;

  HDLGen::HDLPortType *GetHDLType();
};

/*
A fixed length array
*/
class ArrayType : public DataType {
public:
  ArrayType(DataType *_baseType, int _length);

  DataType *baseType; // Type that the array contains
  int length;         // Fixed length of the array

  string GetName();
  int GetWidth();
  bool Equals(DataType *other);
  vector<int> GetDimensions();
  DataType *GetBaseType();

  HDLGen::HDLPortType *GetHDLType();
};
/*
A stream or stream2d
*/
class StreamType : public DataType {
public:
  StreamType(DataType *_baseType, bool _2d, int _length, int _height = -1,
             int _lineWidth = -1);

  DataType *baseType; // Type that the array contains
  bool isStream2d = false;
  int length;    // Fixed length of the array
  int height;    // Height of the array
  int lineWidth; // Width for line buffer fifo

  string GetName();
  int GetWidth();
  bool Equals(DataType *other);
  vector<int> GetDimensions();
  DataType *GetBaseType();

  HDLGen::HDLPortType *GetHDLType();
};

// An entry in a datastructure
struct DataStructureItem {
public:
  DataStructureItem();
  DataStructureItem(string _name, DataType *_type);
  string name;
  DataType *type;
};
bool operator==(const DataStructureItem &a, const DataStructureItem &b);
/*
A user-specified datastructure
*/
class StructureType : public DataType {
public:
  StructureType();
  StructureType(string _name, const vector<DataStructureItem> &_content);

  string structName;
  vector<DataStructureItem> content;

  string GetName();
  int GetWidth();
  bool Equals(DataType *other);
  vector<int> GetDimensions();
  DataType *GetMemberType(string member);

  HDLGen::HDLPortType *GetHDLType();
};

/*
A RAM device - currently can only contain integers
*/
class RAMType : public DataType {
public:
  RAMType(IntegerType _baseType, int _length);
  IntegerType baseType;
  int length;

  string GetName();
  int GetWidth();
  bool Equals(DataType *other);
  vector<int> GetDimensions();

  DataType *GetBaseType();
  bool is_rom = false;

  HDLGen::HDLPortType *GetHDLType();
};

class DeclaredConstant;

// Convert a stream/stream2d/ram type to the closest possible array equivalent
DataType *GetArrayEquivalent(DataType *original);
// Return the ultimate base type of an array
DataType *GetUltimateBase(DataType *array);
// Get 'compatability score' of two types, for operator overloading purposes, or
// -1 if they are completely incompatible
// This assumes  GetArrayEquivalent has been called first on passed_argument
int GetCompatabilityScore(DataType *passed_argument, DataType *argument_type);
// Returns the effective data type of a declared constant
DataType *GetDeclaredConstantType(DeclaredConstant *cnst);
}
