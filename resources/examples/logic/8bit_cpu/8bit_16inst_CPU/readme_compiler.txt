
There is a compiler in simulide for this kind of CPU projects.
It will generate a data file that can be uploaded to program memory.
It can be used for different CPUs (you must provide opcodes).

- Program memory is limited to 256 bytes.
- Must be a "NOP" instruction.
- One ASM line should be (see example): Address Instruction Argument
- Only one argument supported.
- File extension = .sac
- Opcodes must be in a file named "opcodes"
- Fisr line of opcode file must be opcode lenght in bits (see example).


Program example:

0  NOP
1  STM 1
2  LDI 4
3  ADD 1
4  JPI 1



Opcode file example:

codeBits = 4

INC = 0
RLA = 1
ADD = 2
SUB = 3
AND = 4
OR =  5
XOR = 6
NOT = 7

LDI = 8
LDM = 9
STM = 10
JPI = 11
JPZ = 12
JPC = 13
JPN = 14
NOP = 15




How to use:

- Open .sac file in Editor.
- Select folder containing opcodes file as compiler path.
- Compile.
- load generate .data file to program memory.
- Run.
