#pragma once
#include <string>
using namespace std;

namespace RapidHLS {
namespace HDLGen {

class HDLPortType {
public:
  virtual string GetVHDLType() const = 0;
  virtual int GetWidth() const = 0;

  virtual string VHDLCastFrom(const HDLPortType *other,
                              const string &value) const;
};

class LogicSignalPortType : public HDLPortType {
public:
  virtual string GetVHDLType() const;
  virtual int GetWidth() const;
  virtual string VHDLCastFrom(const HDLPortType *other,
                              const string &value) const;
};

class ClockSignalPortType : public LogicSignalPortType {};

class LogicVectorPortType : public HDLPortType {
public:
  LogicVectorPortType(int _width);
  virtual string GetVHDLType() const;
  virtual int GetWidth() const;
  virtual string VHDLCastFrom(const HDLPortType *other,
                              const string &value) const;

private:
  int width;
};

class NumericPortType : public HDLPortType {
public:
  NumericPortType(int _width, bool _signed);
  virtual string GetVHDLType() const;
  virtual int GetWidth() const;
  virtual string VHDLCastFrom(const HDLPortType *other,
                              const string &value) const;

private:
  int width;
  bool is_signed;
};
}
}
