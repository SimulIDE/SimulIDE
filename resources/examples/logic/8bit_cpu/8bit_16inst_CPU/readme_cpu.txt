
8 Bit CPU:

- Data RAM: 256 words 8 bit
- Prog ROM: 256 words 12 bit
- Combinational Control Unit.
- 12 bit intructions: 4 bit opcode + 8 bit argument
- 16 single cycle Instructions:


DEC HEX  BIN  NAME CONTROL   DESCRIPTION

00  0   0000  INC  -------   A + 1
01  1   0001  RLA  -------   ROTATE LEFT  A
02  2   0010  ADD  REM+LDB   A  +  RAM[I]
03  3   0011  SUB  REM+LDB   A  -  RAM[I]
04  4   0100  AND  REM+LDB   A AND RAM[I]
05  5   0101  OR   REM+LDB   A OR  RAM[I]
06  6   0110  XOR  REM+LDB   A XOR RAM[I]
07  7   0111  NOT  -------   NOT A

08  8   1000  LDI  OUI+LDA   A LOAD I
09  9   1001  LDM  REM+LDA   A LOAD RAM[I]
10  A   1010  STM  OUY+WRM   RAM[I] STORE Y
11  B   1011  JPI  JPI       JUMP I
12  C   1100  JPZ  JPZ       JUMP I IF Z
13  D   1101  JPC  JPC       JUMP I IF C
14  E   1110  JPN  JPN       JUMP I IF N
15  F   1111  NOP            NO OPERATION

----------------------------------------

CONTROL Outs:

OUI - I to Data Bus
OUY - Y to Data Bus
LDB - Data Bus to B
LDA - Data Bus to A
WRM - Data Bus to RAM[I]
REM - RAM[I] to Data Bus
JPI - JUMP I
JPZ - JUMP I IF Z
JPC - JUMP I IF C
JPN - JUMP I IF N

