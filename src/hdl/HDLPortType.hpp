#pragma once
#include <string>
using namespace std;

namespace RapidHLS {
namespace HDLGen {

class HDLPortType {
public:
  virtual string GetVHDLType() const = 0;
  virtual int GetWidth() const = 0;
  virtual bool IsSigned() const;
  virtual string VHDLCastFrom(const HDLPortType *other,
                              const string &value) const;
  virtual string GetZero() const = 0;
};

class LogicSignalPortType : public HDLPortType {
public:
  virtual string GetVHDLType() const;
  virtual int GetWidth() const;
  virtual string VHDLCastFrom(const HDLPortType *other,
                              const string &value) const;
  virtual string GetZero() const;
};

class ClockSignalPortType : public LogicSignalPortType {};

class LogicVectorPortType : public HDLPortType {
public:
  LogicVectorPortType(int _width);
  virtual string GetVHDLType() const;
  virtual int GetWidth() const;
  virtual string VHDLCastFrom(const HDLPortType *other,
                              const string &value) const;
  virtual string GetZero() const;

private:
  int width;
};

class NumericPortType : public HDLPortType {
public:
  NumericPortType(int _width, bool _signed);
  virtual string GetVHDLType() const;
  virtual int GetWidth() const;
  virtual bool IsSigned() const;
  virtual string VHDLCastFrom(const HDLPortType *other,
                              const string &value) const;
  virtual string GetZero() const;

private:
  int width;
  bool is_signed;
};
}
}
