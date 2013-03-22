//	File: GenericKeyboard.h
//
//	Author: Colin Heinzmann
//	Description: Emulates the generic keyboard as specified at dcpu.com/keyboard

//	NOTE:  Only partial keyboard support (thats kinda been hacked together).
//		   Shift is a little funky and only the A-Z and 1-9 keys are mapped

#ifndef GENERIC_KEYBOARD_H
#define GENERIC_KEYBOARD_H

#include "DCPUHardware.h"

#include <windows.h>

#define GK_BACK			0x10
#define GK_RETURN		0x11
#define GK_INSERT		0x12
#define	GK_DELETE		0x13
#define GK_ARROW_UP		0x80
#define GK_ARROW_DOWN	0x81
#define GK_ARROW_LEFT	0x82
#define GK_ARROW_RIGHT	0x83
#define GK_SHIFT		0x90
#define GK_CONTROL		0x91

class GenericKeyboard : public DCPUHardware
{
private:
	char	buffer[16];		// the keyboard has a built-in default keyboard
	UINT	bPosition;		// holds the position in the buffer

	BOOL	isSendingInterrupts;	// flag to see if keyboard needs to send interrupts

	BOOL	isShiftDown;			// flag to see if shift is held down

public:
	GenericKeyboard(void* cpuPtr);

	virtual void InterruptHandler(UINT16* DCPURegisters);

	BOOL KeyDown(WPARAM wParam, LPARAM lParam);
	BOOL KeyUp(WPARAM wParam, LPARAM lParam);

private:
	BOOL AddToBuffer(char toAdd);
};

#endif
