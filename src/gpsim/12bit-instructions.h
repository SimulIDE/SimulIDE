/*
   Copyright (C) 1998 T. Scott Dattalo

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


#ifndef __12BIT_INSTRUCTIONS_H__
#define __12BIT_INSTRUCTIONS_H__

#include "pic-instructions.h"

//---------------------------------------------------------
class ADDWF : public Register_op
{
public:

  ADDWF(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new ADDWF(new_cpu,new_opcode,address);}
};

//---------------------------------------------------------

class ANDLW : public Literal_op
{

public:
  ANDLW(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new ANDLW(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class ANDWF : public Register_op
{
public:

  ANDWF(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new ANDWF(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class BCF : public Bit_op
{
public:

  BCF(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();

  static instruction *construct(Processor *new_cpu, uint new_opcode,uint address)
    {return new BCF(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class BSF : public Bit_op
{
public:

  BSF(Processor *new_cpu, uint new_opcode,uint address);
  virtual void execute();

  static instruction *construct(Processor *new_cpu, uint new_opcode,uint address)
    {return new BSF(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class BTFSC : public Bit_op
{
public:

  BTFSC(Processor *new_cpu, uint new_opcode,uint address);
  virtual void execute();

  static instruction *construct(Processor *new_cpu, uint new_opcode,uint address)
    {return new BTFSC(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class BTFSS : public Bit_op
{
public:

  BTFSS(Processor *new_cpu, uint new_opcode,uint address);
  virtual void execute();

  static instruction *construct(Processor *new_cpu, uint new_opcode,uint address)
    {return new BTFSS(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class CALL : public instruction
{
public:
  uint destination;

  CALL(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  virtual char *name(char *str,int len);
  virtual bool isBase() { return true;}

  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new CALL(new_cpu,new_opcode,address);}
};

//---------------------------------------------------------
class CLRF : public Register_op
{
public:

  CLRF(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  virtual char *name(char *str,int len);
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new CLRF(new_cpu,new_opcode,address);}
};

//---------------------------------------------------------
class CLRW : public instruction
{
public:

  CLRW(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  virtual bool isBase() { return true;}
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new CLRW(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class CLRWDT : public instruction
{
public:

  CLRWDT(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  virtual bool isBase() { return true;}
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new CLRWDT(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class COMF : public Register_op
{
public:

  COMF(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new COMF(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class DECF : public Register_op
{
public:

  DECF(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new DECF(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class DECFSZ : public Register_op
{
public:

  DECFSZ(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new DECFSZ(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class GOTO : public instruction
{
public:
  uint destination;

  GOTO(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  virtual bool isBase() { return true;}
  virtual char *name(char *str,int len);
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new GOTO(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class INCF : public Register_op
{
public:

  INCF(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new INCF(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class INCFSZ : public Register_op
{
public:

  INCFSZ(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new INCFSZ(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------

class IORLW : public Literal_op
{

public:
  IORLW(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new IORLW(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class IORWF : public Register_op
{
public:

  IORWF(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new IORWF(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class MOVF : public Register_op
{
public:

  MOVF(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  virtual void debug();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new MOVF(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------

class MOVLW : public Literal_op
{
public:
  MOVLW(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new MOVLW(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class MOVWF : public Register_op
{
public:

  MOVWF(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  virtual char *name(char *str,int len);
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new MOVWF(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class NOP : public instruction
{
public:

  NOP(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  virtual bool isBase() { return true;}
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new NOP(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class OPTION : public instruction
{
public:

  OPTION(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  virtual bool isBase() { return true;}
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new OPTION(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------

class RETLW : public Literal_op
{
public:

  RETLW(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new RETLW(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class RLF : public Register_op
{
public:

  RLF(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new RLF(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class RRF : public Register_op
{
public:

  RRF(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new RRF(new_cpu,new_opcode,address);}

};


//---------------------------------------------------------
class SLEEP : public instruction
{
public:

  SLEEP(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  virtual bool isBase() { return true;}
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new SLEEP(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class SUBWF : public Register_op
{
public:

  SUBWF(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new SUBWF(new_cpu,new_opcode,address);}

};


//---------------------------------------------------------
class SWAPF : public Register_op
{
public:

  SWAPF(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();

  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new SWAPF(new_cpu,new_opcode,address);}

};


//---------------------------------------------------------
class TRIS : public Register_op
{
public:
  Register *reg;

  TRIS(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  virtual char *name(char *str,int len);
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new TRIS(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------

class XORLW : public Literal_op
{

public:

  XORLW(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new XORLW(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class XORWF : public Register_op
{
public:

  XORWF(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new XORWF(new_cpu,new_opcode,address);}

};


#endif  /*  __12BIT_INSTRUCTIONS_H__ */
