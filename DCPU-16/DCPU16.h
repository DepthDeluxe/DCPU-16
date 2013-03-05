//	File: DCPU16.h
//
//	Author: Colin Heinzmann
//	Description: This is an emulator for 

#ifndef DCPU16_H
#define DCPU16_H

#include <windows.h>
#include <vector>

#include "DCPUHardware.h"

using namespace std;

// based off of 1.7 specs online
#define DCPU_VER_HIBYTE	1
#define DCPU_VER_LOBYTE	7

#pragma region DCPU_SPECS

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

#pragma endregion

#define	INSTRUCTION	UINT16

enum InterruptStatus { NONE, IN_PROGRESS, BREAK };

class DCPU16
{
private:
	// 0x10000 words of ram memory
	UINT16	Ram[0x10000];
	
	UINT16	Register[8];		// holds pointers to locations in ram (A,B,C,X,Y,Z,I,J)
	UINT16	ProgramCounter;		// holds address of next instruction
	UINT16	StackPointer;		// holds current position of the stack (in terms of DCPU ram location)
	UINT16	Extra;				// extra register for carry-overs on some operations
	UINT16	InterruptAddress;	// holds the location of the interrupt service routine in memory

	InterruptStatus	interruptStatus;		// flag to see if an interrupt has been requested on the hardware
	BOOL			isQueueingInterrupts;	// if this is true, all interrupts get stored in the interrupt queue
	BOOL			isIgnoringInterrupts;	// if true, then interrupts will be ignored

	vector<UINT16>	InterruptQueue;			// holds the function pointer for start of interrupt

	vector<DCPUHardware*> attachedHardware; // holds information about attached hardware

	double	ClockCounter;		// should only be incremented by 1 whole number but
								// needs to go to higher range than just 3 million
								// (just in case)

	BOOL	SkipInstruction;	// if flag enabled, the DPCU skips operating on
								// the next instruction.  For IF statements
	BOOL	isHalted;			// if flag enabled, DCPU is halted, preventing
								// code from resuming

public:
	DCPU16();

	BOOL NextInstruction();

	// debugging functions
	void SetRam(int location, int length, UINT16* data);		// writes a certain length of data to the location given in ram memory
	void ReadRam(int location, int length, UINT16* out);		// reads ram memory
	UINT16* GetRamPtr();

	void SetRegister(int location, UINT16 value);	// handles registers
	UINT16 GetRegister(int location);

	void ConnectHardware(DCPUHardware* dHard);		// hardware links
	BOOL TriggerInterrupt(UINT16 message);

	void SetPC(int location);
	UINT16 GetPC();

	double GetClockCount();				// clock simulation handler (not yet implemented)
	BOOL IsHalted();

	void DumpMemory(char* filename);	// dumps the ram memory into the file specified

private:
	void Clock();						// used to clock the system to simulate real processing speed (not implemented)
	void Clock(UINT numClocks);

	void	Push(UINT16 valueToPush);	// push and pop stack operators
	UINT16	Pop();						//
};

#endif
