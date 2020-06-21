/*
   Copyright (C) 1998-2003 Scott Dattalo

This file is part of the libgpsim library of gpsim

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, see
<http://www.gnu.org/licenses/lgpl-2.1.html>.
*/


#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <typeinfo>

#include "processor.h"
#include "value.h"
#include "errors.h"
#include "protocol.h"
#include "config.h"



char * TrimWhiteSpaceFromString(char * pBuffer) {
  size_t iPos = 0;
  char * pChar = pBuffer;
  while(*pChar != 0 && ::isspace(*pChar)) {
    pChar++;
  }
  if(pBuffer != pChar) {
    memmove(pBuffer, pChar, strlen(pBuffer) - iPos);
  }
  iPos = strlen(pBuffer);
  if(iPos > 0) {
    pChar = pBuffer + iPos - 1;
    while(pBuffer != pChar && ::isspace(*pChar)) {
      *pChar = 0;
      pChar--;
    }
  }
  return pBuffer;
}

char * UnquoteString(char * pBuffer) {
  char cQuote;
  if(*pBuffer == '\'') {
    cQuote = '\'';
  }
  else if(*pBuffer == '"') {
    cQuote = '"';
  }
  else {
    return pBuffer;
  }
  int iLen = strlen(pBuffer);
  if(iLen > 1) {
    if(pBuffer[iLen - 1] == cQuote) {
      memmove(&pBuffer[0], &pBuffer[1], iLen - 2);
      pBuffer[iLen - 2] = 0;
    }
  }
  return pBuffer;
}

string &toupper(string & sStr) {
  string::iterator it;
  string::iterator itEnd = sStr.end();
  for(it = sStr.begin(); it != itEnd; ++it) {
    if(isalpha(*it)) {
      *it = toupper((int)*it);
    }
  }
  return sStr;
}

//------------------------------------------------------------------------
Value::Value()
  : cpu(0)
{
}

Value::Value(const char *_name, const char *desc, Module *pMod)
  : gpsimObject(_name,desc), cpu(pMod)
{
}

Value::~Value()
{
}
void Value::update()
{
}

void Value::set(const char *cP,int i)
{
  throw new Error(" cannot assign string to a " + showType());
}
void Value::set(double d)
{
  throw new Error(" cannot assign a double to a " + showType());
}
void Value::set(int64_t i)
{
  throw new Error(" cannot assign an integer to a " + showType());
}
void Value::set(bool v)
{
  throw new Error(" cannot assign a boolean to a " + showType());
}

void Value::set(int i)
{
  int64_t i64 = i;
  set(i64);
}

void Value::set(Value *v)
{
  throw new Error(" cannot assign a Value to a " + showType());
}

void Value::set(Packet &pb)
{
  cout << "Value,"<<name()<<" is ignoring packet buffer for set()\n";
}

void Value::get(int64_t &i)
{
  throw new Error(showType() + " cannot be converted to an integer ");
}

void Value::get(int &i)
{
  int64_t i64;
  get(i64);
  i = (int) i64;
}

void Value::get(uint64_t &i)
{
  // FIXME - casting a signed int to an uint -- probably should issue a warning
  int64_t i64;
  get(i64);
  i = (int64_t) i64;
}

void Value::get(bool &b)
{
  throw new Error(showType() +
                  " cannot be converted to a boolean");
}

void Value::get(double &d)
{
  throw new Error(showType() +
                  " cannot be converted to a double ");
}

// get as a string - no error is thrown if the derived class
// does not provide a method for converting to a string -
// instead we'll return a bogus value.

void Value::get(char *buffer, int buf_size)
{
  if(buffer)
    strncpy(buffer,"INVALID",buf_size);
}

void Value::get(Packet &pb)
{
  cout << "Value,"<<name()<<" is ignoring packet buffer for get()\n";
}

Value *Value::copy()
{
  throw new Error(" cannot copy " + showType());
}

Processor *Value::get_cpu() const
{
  return static_cast<Processor *>(cpu);
}

void Value::set_cpu(Processor *new_cpu)
{
  cpu = new_cpu;
}
void Value::set_module(Module *new_cpu)
{
  cpu = new_cpu;
}
Module *Value::get_module()
{
  return cpu;
}

void Value::addName(string &r_sAliasedName)
{
}

//------------------------------------------------------------------------
ValueWrapper::ValueWrapper(Value *pCopy)
  : m_pVal(pCopy)
{
}
ValueWrapper::~ValueWrapper()
{
}
uint ValueWrapper::get_leftVal()
{
  return m_pVal->get_leftVal();
}
uint ValueWrapper::get_rightVal()
{
  return m_pVal->get_rightVal();
}
void ValueWrapper::set(const char *cP,int len)
{
  m_pVal->set(cP,len);
}
void ValueWrapper::set(double d)
{
  m_pVal->set(d);
}
void ValueWrapper::set(int64_t i)
{
  m_pVal->set(i);
}
void ValueWrapper::set(int i)
{
  m_pVal->set(i);
}
void ValueWrapper::set(bool b)
{
  m_pVal->set(b);
}
void ValueWrapper::set(Value *v)
{
  m_pVal->set(v);
}
void ValueWrapper::set(Packet &p)
{
  m_pVal->set(p);
}
void ValueWrapper::get(bool &b)
{
  m_pVal->get(b);
}
void ValueWrapper::get(int &i)
{
  m_pVal->get(i);
}
void ValueWrapper::get(uint64_t &i)
{
  m_pVal->get(i);
}
void ValueWrapper::get(int64_t &i)
{
  m_pVal->get(i);
}
void ValueWrapper::get(double &d)
{
  m_pVal->get(d);
}
void ValueWrapper::get(char *pC, int len)
{
  m_pVal->get(pC,len);
}
void ValueWrapper::get(Packet &p)
{
  m_pVal->get(p);
}
Value *ValueWrapper::copy()
{
  return m_pVal->copy();
}
void ValueWrapper::update()
{
  m_pVal->update();
}
Value *ValueWrapper::evaluate()
{
  return m_pVal->evaluate();
}

/*****************************************************************
 * The AbstractRange class.
 */
AbstractRange::AbstractRange(uint newLeft, uint newRight)
{
  left = newLeft;
  right = newRight;
}

AbstractRange::~AbstractRange()
{
}

string AbstractRange::toString()
{
  char buff[256];

  snprintf(buff, sizeof(buff), "%u:%u", left, right);

  return (string(buff));
}

string AbstractRange::toString(const char* format)
{
  char cvtBuf[1024];

  snprintf(cvtBuf, sizeof(cvtBuf), format, left, right);
  return (string(&cvtBuf[0]));
}

char *AbstractRange::toString(char *return_str, int len)
{
  if(return_str) {
    snprintf(return_str, len, "%u:%u", left, right);
  }

  return return_str;
}

uint AbstractRange::get_leftVal()
{
  return(left);
}

uint AbstractRange::get_rightVal()
{
  return(right);
}

AbstractRange* AbstractRange::typeCheck(Value* val, string valDesc)
{
  if (typeid(*val) != typeid(AbstractRange)) {
    throw new TypeMismatch(valDesc, "AbstractRange", val->showType());
  }
  // This static cast is totally safe in light of our typecheck, above.
  return((AbstractRange*)(val));
}

Value *AbstractRange::copy()
{
  return new AbstractRange(get_leftVal(),get_rightVal());
}

void AbstractRange::set(Value *v)
{
  AbstractRange *ar=typeCheck(v, string(""));
  left = ar->get_leftVal();
  right = ar->get_rightVal();
}


/*****************************************************************
 * The Boolean class.
 */
Boolean::Boolean(bool newValue)
{
  value = newValue;
}

Boolean::Boolean(const char *_name, bool newValue, const char *_desc)
  : Value(_name,_desc)
{
  value = newValue;

}

bool Boolean::Parse(const char *pValue, bool &bValue) {
  if(strncmp("true", pValue, sizeof("true")-1) == 0) {
    bValue = true;
    return true;
  }
  else if(strncmp("false", pValue, sizeof("false")-1) == 0) {
        bValue = false;
    return true;
  }
  return false;
}

Boolean * Boolean::NewObject(const char *_name, const char *pValue, const char *desc) {
  bool bValue;
  if(Parse(pValue, bValue)) {
    return new Boolean(_name, bValue);
  }
  return NULL;
}

Boolean::~Boolean()
{
}

string Boolean::toString()
{
  bool b;
  get(b);
  return (string(b ? "true" : "false"));
}

string Boolean::toString(bool value)
{
  return (string(value ? "true" : "false"));
}

char *Boolean::toString(char *return_str, int len)
{
  if(return_str) {
    bool b;
    get(b);
    snprintf(return_str,len,"%s",(b ? "true" : "false"));
  }

  return return_str;
}
char *Boolean::toBitStr(char *return_str, int len)
{
  if(return_str) {
    bool b;
    get(b);
    snprintf(return_str,len,"%d",(b ? 1 : 0));
  }

  return return_str;
}

string Boolean::toString(const char* format)
{
  char cvtBuf[1024];
  bool b;
  get(b);

  snprintf(cvtBuf, sizeof(cvtBuf), format, b);
  return cvtBuf;
}

Boolean* Boolean::typeCheck(Value* val, string valDesc)
{
  if (typeid(*val) != typeid(Boolean)) {
    throw new TypeMismatch(valDesc, "Boolean", val->showType());
  }

  // This static cast is totally safe in light of our typecheck, above.
  return((Boolean*)(val));
}

Value *Boolean::copy()
{
  bool b;
  get(b);
  return new Boolean(b);
}

// get(bool&) - primary method for accessing the value.
void Boolean::get(bool &b)
{
  b = value;
}

// get(int&) - type cast an integer into a boolean. Note
// that we call get(bool &) instead of directly accessing
// the member value. The reason for this is so that derived
// classes can capture the access.
void Boolean::get(int &i)
{
  bool b;
  get(b);
  i = b ? 1 : 0;
}
/*
void Boolean::get(double &d)
{
  bool b;
  get(b);
  d = b ? 1.0 : 0.0;
}
*/
void Boolean::get(char *buffer, int buf_size)
{
  if(buffer) {

    bool b;
    get(b);
    if(b)
      strncpy(buffer,"true",buf_size);
    else
      strncpy(buffer,"false",buf_size);
  }

}
void Boolean::get(Packet &pb)
{
  bool b;
  get(b);
  pb.EncodeBool(b);
}

void Boolean::set(Value *v)
{
  Boolean *bv = typeCheck(v,string("set "));
  bool b = bv->getVal();
  set(b);
}

void Boolean::set(bool v)
{
  value = v;
  //if(get_xref())
  //  get_xref()->set(v);
}

void Boolean::set(const char *buffer, int buf_size)
{
  if(buffer) {
    bool bValue;
    if(Parse(buffer, bValue)) {
      set(bValue);
    }
  }
}

void Boolean::set(Packet &p)
{
  bool b;
  if(p.DecodeBool(b))
    set(b);
}


/*****************************************************************
 * The Integer class.
 */
Integer::Integer( const Integer &new_value ) 
       : Value()
{
  Integer & nv = (Integer&)new_value;
  nv.get(value);
  bitmask = new_value.bitmask;
}

Integer::Integer(int64_t newValue)
{
  value = newValue;
  bitmask = def_bitmask;
}

Integer::Integer(const char *_name, int64_t newValue,const char *_desc)
  : Value(_name,_desc)
{
  value = newValue;
  bitmask = def_bitmask;
}

int64_t Integer::def_bitmask = 0xffffffff;

Integer::~Integer()
{
}

void Integer::setDefaultBitmask(int64_t bitmask) 
{
  def_bitmask = bitmask;
}

Value *Integer::copy()
{
  int64_t i;
  get(i);
  return new Integer(i);
}

void Integer::set(double d)
{
  int64_t i = (int64_t)d;
  set(i);
}

void Integer::set(int64_t i)
{
  value = i;
  //if(get_xref())
  //  get_xref()->set(i);
}
void Integer::set(int i)
{
  int64_t ii = i;
  set(ii);
}
void Integer::set(Value *v)
{
  int64_t iv = 0;
  if (v)  v->get(iv);

  set(iv);
}

void Integer::set(Packet &p)
{
  uint i;
  if(p.DecodeUInt32(i)) 
  {
    set((int)i);
    return;
  }

  uint64_t i64;
  if(p.DecodeUInt64(i64)) 
  {
    set((int64_t)i64);
    return;
  }
}

void Integer::set(const char *buffer, int buf_size)
{
  if(buffer) 
  {
    int64_t i;
    if(Parse(buffer, i)) set(i);
  }
}

bool Integer::Parse(const char *pValue, int64_t &iValue) {
    if(::isdigit(*pValue)) 
    {
      if(strchr(pValue, '.'))  return false;
      else          return sscanf(pValue, "%li", &iValue) == 1;
    }
    else if(*pValue == '$' && ::isxdigit(*(pValue+1))) 
    {
      // hexidecimal integer
      char szHex[10] = "0x";
      strcat(&szHex[0], pValue + 1);
      return sscanf(szHex, "%li" , &iValue) == 1;
    }
    return false;
}

Integer * Integer::NewObject(const char *_name, const char *pValue, const char *desc) {
  int64_t iValue;
  if(Parse(pValue, iValue)) {
    return new Integer(_name, iValue, desc);
  }
  return NULL;
}


void Integer::get(int64_t &i)
{
  i = value;
}

void Integer::get(double &d)
{
  int64_t i;
  get(i);
  d = (double)i;
}

void Integer::get(char *buffer, int buf_size)
{
  if(buffer) {

    int64_t i;
    get(i);
    long long int j = i;
    snprintf(buffer,buf_size,"%" PRINTF_INT64_MODIFIER "d",j);
  }

}
void Integer::get(Packet &pb)
{
  int64_t i;
  get(i);

  uint j = (uint) (i &0xffffffff);
  pb.EncodeUInt32(j);
}

string Integer::toString()
{
  return "";
}

string Integer::toString(const char* format)
{
  char cvtBuf[1024];

  int64_t i;
  get(i);

  snprintf(cvtBuf,sizeof(cvtBuf), format, i);
  return (string(&cvtBuf[0]));
}


string Integer::toString(const char* format, int64_t value)
{
  char cvtBuf[1024];

  snprintf(cvtBuf,sizeof(cvtBuf), format, value);
  return (string(&cvtBuf[0]));
}

string Integer::toString(int64_t value)
{
  char cvtBuf[1024];
  long long int v=value;
  snprintf(cvtBuf,sizeof(cvtBuf), "%" PRINTF_INT64_MODIFIER "d", v);
  return (string(&cvtBuf[0]));
}

char *Integer::toString(char *return_str, int len)
{
  return return_str;
}
char *Integer::toBitStr(char *return_str, int len)
{
  if(return_str) {
    int64_t i;
    get(i);
    int j=0;
    int mask=1<<31;
    for( ; mask ; mask>>=1, j++)
      if(j<len) return_str[j] = ( (i & mask) ? 1 : 0);

    if(j<len) return_str[j]=0;
  }
  return return_str;
}

Integer* Integer::typeCheck(Value* val, string valDesc)
{
  if (typeid(*val) != typeid(Integer)) {
    throw new TypeMismatch(valDesc, "Integer", val->showType());
  }

  // This static cast is totally safe in light of our typecheck, above.
  return((Integer*)(val));
}

Integer* Integer::assertValid(Value* val, string valDesc, int64_t valMin)
{
  Integer* iVal;
  int64_t i;

  iVal = Integer::typeCheck(val, valDesc);
  iVal->get(i);

  if (i < valMin) {
    throw new Error(valDesc +
                    " must be greater than " + Integer::toString(valMin) +
                    ", saw " + Integer::toString(i)
                    );
  }

  return(iVal);
}

Integer* Integer::assertValid(Value* val, string valDesc, int64_t valMin, int64_t valMax)
{
  Integer* iVal;
  int64_t i;

  iVal = (Integer::typeCheck(val, valDesc));

  iVal->get(i);

  if ((i < valMin) || (i>valMax)) {
    throw new Error(valDesc +
                    " must be be in the range [" + Integer::toString(valMin) + ".." +
                    Integer::toString(valMax) + "], saw " + Integer::toString(i)
                    );
  }

  return(iVal);
}

/*****************************************************************
 * The Float class.
 */
Float::Float(double newValue)
{
  value = newValue;
}

Float::Float(const char *_name, double newValue,const char *_desc)
  : Value(_name,_desc)
{
  value = newValue;
}

bool Float::Parse(const char *pValue, double &fValue)
{
  return pValue ? sscanf(pValue,"%lg",&fValue) == 1 : false;
}

Float * Float::NewObject(const char *_name, const char *pValue, const char *desc) {
  double fValue;
  if(Parse(pValue, fValue)) {
    return new Float(_name, fValue);
  }
  return NULL;
}

Float::~Float()
{
}

void Float::set(double d)
{
  value = d;
}

void Float::set(int64_t i)
{
  double d = (double)i;
  set(d);
}

void Float::set(Value *v)
{
  /* typeCheck means cannot set integers - RRR
  Float *fv = typeCheck(v,string("set "));
  double d = fv->getVal();
  set(d);
  */
   double d;

  if (typeid(*v) != typeid(Float) &&
      typeid(*v) != typeid(Integer))
  {
    throw new TypeMismatch(string("set "), "Float", v->showType());
  }
   v->get(d);
   set(d);
}

void Float::set(const char *buffer, int buf_size)
{
  if(buffer) {

    double d;
    if(Parse(buffer, d)) {
      set(d);
    }
  }
}

void Float::set(Packet &p)
{
  double d;
  if(p.DecodeFloat(d)) {

    set(d);
  }

}

void Float::get(int64_t &i)
{
  double d;
  get(d);
  i = (int64_t)d;
}
void Float::get(double &d)
{
  d = value;
}

void Float::get(char *buffer, int buf_size)
{
  if(buffer) {

    double d;;
    get(d);

    snprintf(buffer,buf_size,"%g",d);
  }

}
void Float::get(Packet &pb)
{
  double d;
  get(d);

  pb.EncodeFloat(d);
}

Value *Float::copy() {
  double d;
  get(d);
  return new Float(d);
}

string Float::toString()
{
  return toString("%#-16.16g");
}


string Float::toString(const char* format)
{
  char cvtBuf[1024];

  double d;
  get(d);

  snprintf(cvtBuf, sizeof(cvtBuf), format, d);
  return cvtBuf;
}

char *Float::toString(char *return_str, int len)
{
  if(return_str) {

    double d;
    get(d);
    snprintf(return_str,len,"%g",d);
  }

  return return_str;
}

Float* Float::typeCheck(Value* val, string valDesc)
{
  if (typeid(*val) != typeid(Float)) {
    throw new TypeMismatch(valDesc, "Float", val->showType());
  }

  // This static cast is totally safe in light of our typecheck, above.
  return((Float*)(val));
}


/*****************************************************************
 * The String class.
 */
String::String(const char *newValue)
{
  if (newValue)
    value = newValue;
}

String::String(const char *newValue, size_t len)
{
  if (newValue)
    value.assign(newValue, len);
}

String::String(const char *_name, const char *newValue, const char *_desc)
  : Value(_name, _desc)
{
  if (newValue)
    value = newValue;
}

String::~String()
{
}

std::string String::toString()
{
  return value;
}

char *String::toString(char *return_str, int len)
{
  if (return_str)
    snprintf(return_str, len, "%s", value.c_str());

  return return_str;
}

void String::set(Value *v)
{
  if (v) {
    std::string buf = v->toString();
    set(buf.c_str());
  }
}

// TODO: is this meant to do something
void String::set(Packet &p)
{
  cout << " fixme String::set(Packet &) is not implemented\n";
}

// TODO: was len meant to do anything
void String::set(const char *s, int len)
{
  if (s)
    value = s;
}

void String::get(char *buf, int len)
{
  if (buf)
    snprintf(buf, len, "%s", value.c_str());
}

void String::get(Packet &p)
{
  p.EncodeString(value.c_str());
}

const char *String::getVal()
{
  return value.c_str();
}

Value *String::copy()
{
  return new String(value.c_str());
}

//------------------------------------------------------------------------
namespace gpsim {
  Function::Function(const char *_name, const char *desc)
    : gpsimObject(_name,desc)
  {
  }

  Function::~Function()
  {
    cout << "Function destructor\n";
  }

  string Function::description()
  {
    if(cpDescription)
      return string(cpDescription);
    else
      return string("no description");
  }

  string Function::toString()
  {
    return name();
  }
}
