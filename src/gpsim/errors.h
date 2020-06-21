#if !defined(_ERRORS_H_)
#define _ERRORS_H_

#include <string>

#include "gpsim_object.h"
using namespace std;

//*****************************************************************
class AnError : public gpsimObject {
 public:
  AnError(const std::string &severity, const std::string &errMsg);
  virtual ~AnError();

  string toString();
  string get_errMsg();

 private:
  string severity;
  string errMsg;
};

//*****************************************************************
class Error : public AnError {
 public:
  explicit Error(const std::string &errMsg);
  virtual ~Error();

  static int count;
};

//*****************************************************************
class FatalError : public AnError {
 public:
  explicit FatalError(const std::string &errMsg);
  virtual ~FatalError();
};

//*****************************************************************
class TypeMismatch : public Error {
 public:
  TypeMismatch(const std::string &theOperator, const std::string &expectedType,
    const std::string &observedType);
  TypeMismatch(const std::string &theOperator, const std::string &observedType);
  virtual ~TypeMismatch();
};

#endif // _ERRORS_
