
#include "errors.h"

/*****************************************************************
 * The primordial Assembler Error class.
 */
AnError::AnError(const std::string & _severity, const std::string & _errMsg)
  : severity(_severity), errMsg(_errMsg)
{
}

AnError::~AnError()
{
}

string AnError::toString()
{
  return string("\"" + errMsg + "\"");
}


string AnError::get_errMsg()
{
  return errMsg;
}

/*****************************************************************
 * Generate assembler errors of severity "ERROR"
 */
int Error::count;

Error::Error(const std::string & errMsg)
  : AnError(string("ERROR"), errMsg)
{
}

Error::~Error()
{
}

/*****************************************************************
 * Generate assembler errors of severity "FATAL_ERROR"
 */
FatalError::FatalError(const std::string & errMsg)
  : AnError(string("FATAL_ERROR"), errMsg)
{
}

FatalError::~FatalError()
{
}

/*****************************************************************
 * Generate a generic Type Mismatch error of the "expected xx,
 * observed yy" variety.
 */
TypeMismatch::TypeMismatch(const std::string &theOperator,
  const std::string &expectedType, const std::string &observedType)
  : Error(" Type mismatch for " + theOperator + " operator. Type expected " + expectedType
          + ", found " + observedType)
{
}

/*****************************************************************
 * Generate a generic Type Mismatch error of the "operator x
 * cannot be applied to type y" variety.
 */
TypeMismatch::TypeMismatch(const std::string &theOperator,
  const std::string &observedType)
  : Error("Operator <" + theOperator + "> cannot be applied to type "
          + observedType)
{
}

TypeMismatch::~TypeMismatch()
{
}
