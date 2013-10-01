//	File: dcpuspecs.h
//
//	Specification by:	Markus Persson
//	Description:		Has all values of defined instructions from the DCPU-16 specification

#ifndef DCPUSPECS
#define DCPUSPECS

#include <windows.h>

///
// Values (5/6 bits) 
///

#define REG_A	0x00
#define REG_B	0x01
#define REG_C	0x02
#define REG_X	0x03
#define REG_Y	0x04
#define REG_Z	0x05
#define REG_I	0x06
#define	REG_J	0x07

#define REG_A_VALUE 0x08
#define REG_B_VALUE 0x09
#define REG_C_VALUE 0x0a
#define REG_X_VALUE 0x0b
#define REG_Y_VALUE	0x0c
#define REG_Z_VALUE 0x0d
#define REG_I_VALUE 0x0e
#define REG_J_VALUE 0x0f

#define REG_A_NEXTWORD	0x10
#define REG_B_NEXTWORD	0x11
#define REG_C_NEXTWORD	0x12
#define REG_X_NEXTWORD	0x13
#define REG_Y_NEXTWORD	0x14
#define REG_Z_NEXTWORD	0x15
#define REG_I_NEXTWORD	0x16
#define	REG_J_NEXTWORD	0x17

#define PUSH					0x18		// PUSH / [--SP] if in b
#define POP						0x18		// POP / [SP++] if in a
#define PEEK					0x19
#define PICK					0x1a
#define	SP						0x1b
#define PC						0x1c
#define EX						0x1d
#define NEXTWORD_VALUE			0x1e
#define NEXTWORD				0x1f

#define NO_OP_DATA				0xff		// marker for DAT


///
// Basic opcodes (5 bits)
///

#define	SET	0x01
#define ADD	0x02
#define SUB	0x03
#define MUL	0x04
#define MLI	0x05
#define DIV	0x06
#define DVI	0x07
#define MOD	0x08
#define MDI	0x09
#define AND	0x0a
#define BOR	0x0b
#define XOR	0x0c
#define SHR	0x0d
#define ASR	0x0e
#define SHL	0x0f
#define IFB	0x10
#define IFC	0x11
#define IFE	0x12
#define IFN	0x13
#define IFG	0x14
#define IFA	0x15
#define IFL	0x16
#define IFU	0x17

#define ADX	0x1a
#define SBX	0x1b

#define STI	0x1e
#define STD	0x1f

///
// Special Opcodes (5 bits)
///

#define JSR		0x01

#define SINT	0x08
#define IAG		0x09
#define IAS		0x0a
#define RFI		0x0b
#define IAQ		0x0c

#define HWN		0x10
#define HWQ		0x11
#define HWI		0x12

// holds the structure of a DCPU instruction
struct DCPU_Instruction
{
	int opcode, a, b;
	int nextA;
	int nextB;

	BOOL isALabel;
	BOOL isBLabel;
};

#endif
