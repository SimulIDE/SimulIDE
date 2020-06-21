/*
   Copyright (C) 1998 T. Scott Dattalo
		 2013 Roy R. Rankin

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

class instruction;  // forward declaration for the include files that follow

#ifndef __14BIT_INSTRUCTIONS_H__
#define __14BIT_INSTRUCTIONS_H__

#define REG_IN_INSTRUCTION_MASK  0x7f
#define DESTINATION_MASK         0x80

#include "pic-instructions.h"
#include "12bit-instructions.h"
#include "14bit-registers.h"


//---------------------------------------------------------
class ADDFSR : public instruction 
{

public:
  ADDFSR(Processor *new_cpu, uint new_opcode,const char *, uint address);
  virtual bool isBase() { return true;}
  virtual void execute();
  virtual char *name(char *,int);
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
  {
    return new ADDFSR(new_cpu,new_opcode,"addfsr", address);
  }
protected:
  uint 	m_fsr;
  int 		m_lit;
  Indirect_Addressing14 *ia;
};

//---------------------------------------------------------

class ADDLW : public Literal_op
{

public:
  ADDLW(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute(void);
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new ADDLW(new_cpu,new_opcode, address);}

};

//---------------------------------------------------------
class ADDWFC : public Register_op
{
public:

  ADDWFC(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
  {return new ADDWFC(new_cpu,new_opcode,address);}
};

//---------------------------------------------------------
class BRA : public instruction
{
public:
  int destination_index;
  uint absolute_destination_index;

  BRA(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  virtual char *name(char *,int);
  virtual bool isBase() { return true;}
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
  {return new BRA(new_cpu,new_opcode,address);}
};

//---------------------------------------------------------
class BRW : public instruction
{
public:
  int destination_index;
  uint current_address;

  BRW(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  virtual char *name(char *,int);
  virtual bool isBase() { return true;}
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
  {return new BRW(new_cpu,new_opcode,address);}
};

//---------------------------------------------------------
class ASRF : public Register_op
{
public:

  ASRF(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
  {return new ASRF(new_cpu,new_opcode,address);}
};

//---------------------------------------------------------
class CALLW : public instruction
{
public:
  CALLW(Processor *new_cpu, uint new_opcode, uint address);
  virtual bool isBase() { return true;}
  virtual void execute();
  virtual char *name(char *,int);
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
  {
    return new CALLW(new_cpu,new_opcode,address);
  }
};

//---------------------------------------------------------
class LSLF : public Register_op
{
public:

  LSLF(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
  {return new LSLF(new_cpu,new_opcode,address);}
};

//---------------------------------------------------------
class LSRF : public Register_op
{
public:

  LSRF(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
  {return new LSRF(new_cpu,new_opcode,address);}
};

//---------------------------------------------------------
class MOVIW : public instruction
{
public:

  MOVIW(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  virtual bool isBase() { return true;}
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
  {return new MOVIW(new_cpu,new_opcode,address);}
  virtual char *name(char *,int);

  enum {
	PREINC,
	PREDEC,
	POSTINC,
	POSTDEC,
	DELTA
  };
protected:
  uint 	m_fsr;
  int 		m_lit;
  uint 	m_op;
  Indirect_Addressing14 *ia;
};


//---------------------------------------------------------
class MOVWI : public instruction
{
public:

  MOVWI(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  virtual bool isBase() { return true;}
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
  {return new MOVWI(new_cpu,new_opcode,address);}
  virtual char *name(char *,int);

  enum {
	PREINC,
	PREDEC,
	POSTINC,
	POSTDEC,
	DELTA
  };
protected:
  uint 	m_fsr;
  int 		m_lit;
  uint 	m_op;
  Indirect_Addressing14 *ia;
};



//---------------------------------------------------------

class MOVLB : public Literal_op
{
public:
  MOVLB(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  virtual char *name(char *return_str,int len);
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
  {return new MOVLB(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------

class MOVLP : public Literal_op
{
public:
  MOVLP(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  virtual char *name(char *return_str,int len);
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
  {return new MOVLP(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class RESET : public instruction
{
public:

  RESET(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  virtual bool isBase() { return true;}
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
  {return new RESET(new_cpu,new_opcode,address);}

};


//---------------------------------------------------------
class RETFIE : public instruction
{
public:

  RETFIE(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute(void);
  virtual bool isBase() { return true;}
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new RETFIE(new_cpu,new_opcode,address);}

};


//---------------------------------------------------------
class RETURN : public instruction
{
public:

  RETURN(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute(void);
  virtual bool isBase() { return true;}
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new RETURN(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------

class SUBLW : public Literal_op
{

public:

  SUBLW(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute(void);
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
    {return new SUBLW(new_cpu,new_opcode,address);}

};

//---------------------------------------------------------
class SUBWFB : public Register_op
{
public:

  SUBWFB(Processor *new_cpu, uint new_opcode, uint address);
  virtual void execute();
  static instruction *construct(Processor *new_cpu, uint new_opcode, uint address)
  {return new SUBWFB(new_cpu,new_opcode,address);}
};



#endif //  __14BIT_INSTRUCTIONS_H__
