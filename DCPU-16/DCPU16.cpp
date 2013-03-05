#ifndef DCPU16_CPP
#define DCPU16_CPP

#include "DCPU16.h"

#include <fstream>
#include <iostream>
#include <math.h>
#include <assert.h>

using namespace std;

DCPU16::DCPU16()
{
	// set program counter to start of memory
	ProgramCounter		= 0;
	StackPointer		= 0xffff;
	Extra				= 0;
	InterruptAddress	= 0;

	// zero the ram memory (to make things easier)
	ZeroMemory(Ram, sizeof(Ram));

	// set other stuff
	ClockCounter	= 0;
	SkipInstruction	= FALSE;
	isHalted			= FALSE;
	
	// init interrupt flags
	interruptStatus = NONE;
	isQueueingInterrupts = FALSE;
	isIgnoringInterrupts = FALSE;
}

BOOL DCPU16::NextInstruction()
{
	// handle interrupts
	if (interruptStatus == NONE && isQueueingInterrupts == FALSE && InterruptQueue.size() > 0)
	{
		// push PC then A onto stack
		this->Push(ProgramCounter);
		this->Push(Register[REG_A]);

		// now set PC to IA and then A to message
		ProgramCounter = InterruptAddress;
		Register[REG_A] = InterruptQueue[0];

		// pop the interrupt queue out
		InterruptQueue.erase(InterruptQueue.begin());

		// remove request to continue normal operation,
		// the interrupt handler should pop the crap
		// off of the stack and reset program counter
		interruptStatus = IN_PROGRESS;
	}

	// allows execution of one instruction before going into another interrupt
	if (interruptStatus == BREAK)
		interruptStatus = NONE;

	// save the initial program counter location
	UINT16 startProgramCounter = ProgramCounter;

	// read memory at PC
	INSTRUCTION instruction = Ram[ProgramCounter];

	// get a, b, and o
	UINT16 binaryA,binaryB,binaryCommand;

	binaryCommand	= instruction & 0x1f;
	binaryB			= (instruction >> 5) & 0x1f;
	binaryA			= (instruction >> 10) & 0x3f;

	// these actually will represent the numbers that will be combined
	UINT16 *valueA, *valueB;		// holders of actual values to be entered
	UINT16 literalValue;

	switch (binaryA)
	{
	case REG_A:
	case REG_B:
	case REG_C:
	case REG_X:
	case REG_Y:
	case REG_Z:
	case REG_I:
	case REG_J:
		valueA = &Register[binaryA];
		break;

	case REG_A_VALUE:
	case REG_B_VALUE:
	case REG_C_VALUE:
	case REG_X_VALUE:
	case REG_Y_VALUE:
	case REG_Z_VALUE:
	case REG_I_VALUE:
	case REG_J_VALUE:
		valueA = &Ram[Register[binaryA]];
		break;

	case REG_A_NEXTWORD:
	case REG_B_NEXTWORD:
	case REG_C_NEXTWORD:
	case REG_X_NEXTWORD:
	case REG_Y_NEXTWORD:
	case REG_Z_NEXTWORD:
	case REG_I_NEXTWORD:
	case REG_J_NEXTWORD:
		ProgramCounter++;
		valueA = &Ram[Register[binaryA-0x10] + Ram[ProgramCounter]];
		break;

	case POP:
		valueA = &Ram[StackPointer];

		// check stack pointer range [0x0000,0xffff]
		if (StackPointer < 0xffff)
			StackPointer++;
		break;

	case PEEK:
		valueA = &Ram[StackPointer];
		break;

	case PICK:
		ProgramCounter++;
		
		if (StackPointer + Ram[ProgramCounter] < 0xffff)
		valueA = &Ram[StackPointer + Ram[ProgramCounter]];
		break;

	case SP:
		valueA = &StackPointer;
		break;

	case PC:
		valueA = &ProgramCounter;
		break;

	case EX:
		valueA = &Extra;
		break;

	case NEXTWORD_VALUE:
		ProgramCounter++;
		valueA = &Ram[Ram[ProgramCounter]];
		break;

	case NEXTWORD:
		ProgramCounter++;
		valueA = &Ram[ProgramCounter];
		break;
	}

	// literal value case 
	if (binaryA >= 0x20 && binaryA <= 0x3f)
	{
		literalValue = binaryA - 0x21;
		valueA = &literalValue;
	}

	if (binaryCommand != 0)
	{
		switch (binaryB)
		{
		case REG_A:
		case REG_B:
		case REG_C:
		case REG_X:
		case REG_Y:
		case REG_Z:
		case REG_I:
		case REG_J:
			valueB = &Register[binaryB];
			break;

		case REG_A_VALUE:
		case REG_B_VALUE:
		case REG_C_VALUE:
		case REG_X_VALUE:
		case REG_Y_VALUE:
		case REG_Z_VALUE:
		case REG_I_VALUE:
		case REG_J_VALUE:
			valueB = &Ram[Register[binaryB]];
			break;

		case REG_A_NEXTWORD:
		case REG_B_NEXTWORD:
		case REG_C_NEXTWORD:
		case REG_X_NEXTWORD:
		case REG_Y_NEXTWORD:
		case REG_Z_NEXTWORD:
		case REG_I_NEXTWORD:
		case REG_J_NEXTWORD:
			ProgramCounter++;
			valueB = &Ram[Register[binaryB-0x10] + Ram[ProgramCounter]];
			break;

		case PUSH:
			// check stack pointer range [0x0000,0xffff]
			if (StackPointer > 0x00 && StackPointer != 0xffff)
				StackPointer--;

			valueB = &Ram[StackPointer];
			break;

		case PEEK:
			valueB = &Ram[StackPointer];
			break;

		case PICK:
			ProgramCounter++;

			if (StackPointer + Ram[ProgramCounter] < 0xffff)
			{
				valueB = &Ram[StackPointer + Ram[ProgramCounter]];
			}
			else
			{
				valueB = &Ram[0xffff];
			}
			break;

		case SP:
			valueB = &StackPointer;
			break;

		case PC:
			valueB = &ProgramCounter;
			break;

		case EX:
			valueB = &Extra;
			break;

		case NEXTWORD_VALUE:
			ProgramCounter++;
			valueB = &Ram[Ram[ProgramCounter]];
			break;

		case NEXTWORD:
			ProgramCounter++;
			valueB = &Ram[ProgramCounter];
			break;
		}
	}

	// don't perform any operations if SkipInstruction has been set
	if (SkipInstruction)
	{
		SkipInstruction = FALSE;
		ProgramCounter++;
		return true;
	}

	// perform standard operations
	switch (binaryCommand)
	{
	case SET:
		// halt the dcpu if program counter is set to its
		// own current value
		if (binaryB == PC && startProgramCounter == *valueA)
			isHalted = TRUE;

		*valueB = *valueA;
		break;

	case ADD:
		*valueB = *valueA + *valueB;
			
		if (*valueA + *valueB > 0xffff)
			Extra = 0x0001;
		else
			Extra = 0x0000;
		break;

	case SUB:
		*valueB = *valueB - *valueA;

		if (*valueA > *valueB)
			Extra = 0xffff;
		else
			Extra = 0;
		break;

	case MUL:
		*valueB = *valueA * *valueB;

		Extra = ((*valueB * *valueA) >> 16) & 0xffff;
		break;

	case MLI:
		*valueB = (UINT16)((int)*valueA * (int)*valueB);

		Extra = (((UINT)((int)*valueB * (int)*valueA)) >> 16) & 0xffff;
		break;

	case DIV:
		// if *valueA is 0, then produce case that gives
		// both set value and extra 0
		if (*valueA == 0)
		{
			*valueA = 1;
			*valueB = 0;
		}

		*valueB = *valueB / *valueA;

		Extra = *valueB % *valueA;
		break;

	case DVI:
		// if *valueA is 0, then produce case that gives
		// both set value and extra 0
		if (*valueA == 0)
		{
			*valueA = 1;
			*valueB = 0;
		}

		*valueB = (UINT16)((int)*valueB / (int)*valueA);

		Extra = (UINT16)((int)*valueB % (int)*valueA);
		break;

	case MOD:
		*valueB = *valueB % *valueA;
		break;

	case MDI:
		*valueB = (UINT16)((int)*valueB % (int)*valueA);
		break;

	case AND:
		*valueB = *valueB & *valueA;
		break;

	case BOR:
		*valueB = *valueB | *valueA;
		break;

	case XOR:
		*valueB = *valueB ^ *valueA;
		break;

	case SHR:
		*valueB = *valueB >> *valueA;
		Extra = *valueB & (UINT)pow(2.0, (double)*valueA);
		break;

	case ASR:
		*valueB = (int)*valueB >> (int)*valueA;
		
		if (*valueA > 0)
			Extra = *valueB & (UINT)pow(2.0, (double)*valueA);
		else
			Extra = (*valueB & (UINT)pow(2.0, (double)(16 - *valueA))) >> (16 - *valueA);

		break;

	case SHL:
		*valueB = *valueB << *valueA;
		Extra = (*valueB & (UINT)pow(2.0, (double)(16 - *valueA))) >> (16 - *valueA);
		break;

	case IFB:
		SkipInstruction = ((*valueB & *valueA) == 0);
		break;

	case IFC:
		SkipInstruction = ((*valueB & *valueA) != 0);
		break;

	case IFE:
		SkipInstruction = (*valueB != *valueA);
		break;

	case IFN:
		SkipInstruction = (*valueB == *valueA);
		break;

	case IFG:
		SkipInstruction = (*valueB <= *valueA);
		break;

	case IFA:
		SkipInstruction = ((int)*valueB <= (int)*valueA);
		break;

	case IFL:
		SkipInstruction = (*valueB >= *valueA);
		break;

	case IFU:
		SkipInstruction = ((int)*valueB >= (int)*valueA);
		break;

	case ADX:
		*valueB = *valueA + *valueB + Extra;

		if (((UINT)*valueB + (UINT)*valueA + (UINT)Extra) > 0xffff)
			Extra = 0x0001;
		else
			Extra = 0x0000;
		break;

	case SBX:
		*valueB = *valueB - *valueA + Extra;

		if ((*valueB - *valueA + (UINT)Extra) > 0xffff)
			Extra = 0x0001;
		else if (((int)*valueB - (int)*valueA + (int)Extra) < 0)
			Extra = 0xffff;
		else
			Extra = 0x0000;
		break;

	case STI:
		*valueB = *valueA;

		Register[REG_I]++;
		Register[REG_J]++;
		break;

	case STD:
		*valueB = *valueA;

		Register[REG_I]--;
		Register[REG_J]--;
		break;
	}

	// now implement as special case scenario
	if (binaryCommand == 0)
	{
		switch(binaryB)
		{
		case JSR:
			// check stack pointer range [0x0000,0xffff]
			// and push the program counter on it
			Push(ProgramCounter);

			// now assign new value to program counter
			ProgramCounter = *valueA;
			break;

		case SINT:
			// perform immediate interrupt
			if (InterruptAddress != 0)
			{
				// push the program counter onto the stack
				Push(ProgramCounter);

				ProgramCounter = InterruptAddress;

				// now turn on queuing
				InterruptAddress = 0;
			}
			break;

		case IAG:
			*valueA = InterruptAddress;
			break;

		case IAS:
			InterruptAddress = *valueA;
			break;

		case RFI:
			// turn off interrupt queuing
			isQueueingInterrupts = FALSE;

			// pop crap off of stack
			Register[0] = Pop();
			ProgramCounter = Pop();

			// set interrupt status to BREAK because the emulator is assuming that
			// RFI is only used to break out of the ISR
			interruptStatus = BREAK;
			break;

		case IAQ:
			// if A is nonzero, interrupts will be added to queue instead of triggered.
			// if zero then interrupts will be triggered as normal again
			if (binaryA == 0)
			{
				isQueueingInterrupts = TRUE;
			}
			else
			{
				isQueueingInterrupts = FALSE;
			}
			break;

		case HWN:
			// sets A to number of connected hardware devices
			Register[REG_A] = (UINT16)attachedHardware.size();
			break;

		case HWQ:
			// sets A, B, C, X, Y registers to info about hardware (not really necessary)
			break;

		case HWI:
			// sends an interrupt to hardware A
			if (*valueA >= 0 && *valueA < attachedHardware.size())
				attachedHardware[*valueA]->InterruptHandler(Register);		// pass a pointer to system registers

			break;
		}
	}

	// if the program counter was assigned to in this operation,
	// don't increase by 1
	if (binaryB == PC || ((binaryB == JSR || binaryB == RFI) && binaryCommand == 0))
		return true;

	// increment the program counter by 1 and continue
	ProgramCounter++;
	return true;
}

void DCPU16::SetRam(int location, int length, UINT16* data)
{
	for (int n = 0; n < length; n++)
		Ram[location + n] = data[n];
}

void DCPU16::ReadRam(int location, int length, UINT16* out)
{
	for (int n = 0; n < length; n++)
		out[n] = Ram[location + n];
}

UINT16* DCPU16::GetRamPtr()
{
	return Ram;
}

void DCPU16::SetRegister(int location, UINT16 value)
{
	Register[location] = value;
}

UINT16 DCPU16::GetRegister(int location)
{
	return Register[location];
}

void DCPU16::ConnectHardware(DCPUHardware* dHard)
{
	attachedHardware.push_back(dHard);
}

BOOL DCPU16::TriggerInterrupt(UINT16 message)
{
	// check to see if interrupts have been ignored
	if (isIgnoringInterrupts)
		return FALSE;

	// add the message to interrupt queue even
	// if queuing disabled, the interrupt should trigger
	// at the beginning of the next cycle
	InterruptQueue.push_back(message);
}

void DCPU16::SetPC(int location)
{
	if (location < 0xffff && location > 0)
		ProgramCounter = location;
}

UINT16 DCPU16::GetPC()
{
	return ProgramCounter;
}

double DCPU16::GetClockCount()
{
	return ClockCounter;
}

BOOL DCPU16::IsHalted()
{
	return isHalted;
}

void DCPU16::DumpMemory(char* filename)
{
	ofstream file;
	file.open(filename, ios::binary);

	// write registers
	file.write("Registers:\r\n", 12);
	for (int n = 0; n < REG_J+1; n++)
		file.write((char*)&Register[n], 2);
	file.write("\r\n\r\n", 4);

	// write ram memory
	file.write("Ram Memory:\r\n", 13);
	for (int n = 0; n < 0x10000; n++)
		file.write((char*)&Ram[n], 2);

	file.close();
}

void DCPU16::Clock()
{
	// perform clocking operation (whatever that will be)
	ClockCounter++;
}

// for clocking multiple times
void DCPU16::Clock(UINT numClocks)
{
	ClockCounter += numClocks;
}

void DCPU16::Push(UINT16 valueToPush)
{
	// check to make sure that (for some miraculous reason,
	// the stack pointer has reached the start of ram
	if (StackPointer > 0)
	{
		StackPointer--;
		Ram[StackPointer] = valueToPush;
	}
}

UINT16 DCPU16::Pop()
{
	if (StackPointer != 0xffff)
		StackPointer++;

	// return the one value above
	return Ram[StackPointer-1];
}

#endif
