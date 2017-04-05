#pragma once
#include <string>
using namespace std;

namespace RapidHLS {
namespace HDLGen {

class HDLPortType {
public:
  virtual string GetVHDLType() = 0;
  virtual string GetVerilogType() = 0;
  virtual int GetWidth() = 0;
};

class LogicSignalPortType : public HDLPortType {
public:
  virtual string GetVHDLType();
  virtual string GetVerilogType();
  virtual int GetWidth();
};

class ClockSignalPortType : public HDLPortType {};

class LogicVectorPortType : public HDLPortType {
public:
  LogicVectorPortType(int _width);
  virtual string GetVHDLType();
  virtual string GetVerilogType();
  virtual int GetWidth();

private:
  int width;
};

class NumericPortType : public HDLPortType {
public:
  NumericPortType(int _width, bool _signed);
  virtual string GetVHDLType();
  virtual string GetVerilogType();
  virtual int GetWidth();

private:
  int width;
  bool is_signed;
};
}
}
