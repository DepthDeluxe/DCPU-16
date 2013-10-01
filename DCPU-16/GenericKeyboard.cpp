#ifndef GENERIC_KEYBOARD_CPP
#define GENERIC_KEYBOARD_CPP

#include "GenericKeyboard.h"
#include "DCPU16.h"

#include <conio.h>

GenericKeyboard::GenericKeyboard(void* cpuPtr)
	: DCPUHardware(cpuPtr)
{
	// zero the character buffer memory
	ZeroMemory(buffer, 16);

	// start at the beginning of the buffer
	bPosition = 0;

	// other crap
	isSendingInterrupts	= FALSE;
	isShiftDown			= FALSE;
}

void GenericKeyboard::InterruptHandler(UINT16* DCPURegisters)
{
	UINT16* regA = DCPURegisters;
	UINT16* regB = &DCPURegisters[1];
	UINT16* regC = &DCPURegisters[2];

	switch(*regA)
	{
	case 0:
		ZeroMemory(buffer,16);
		bPosition = 0;
		break;

	case 1:
		if (bPosition == 0)
			*regC = 0;
		else
		{
			bPosition--;
			*regC = buffer[bPosition];
		}

		break;
	
	case 3:
		if (*regB == 0)
			isSendingInterrupts = FALSE;
		else
			isSendingInterrupts = TRUE;

		interruptMessage = *regB;

		break;
	}
}

BOOL GenericKeyboard::KeyDown(WPARAM wParam, LPARAM lParam)
{
	char convertedKey = NULL;

	switch (wParam)
	{
	case VK_BACK:
		convertedKey = (char)GK_BACK;
		break;

	case VK_RETURN:
		convertedKey = (char)GK_RETURN;
		break;

	case VK_DELETE:
		convertedKey = (char)GK_DELETE;
		break;

	case VK_UP:
		convertedKey = (char)GK_ARROW_UP;
		break;

	case VK_DOWN:
		convertedKey = (char)GK_ARROW_DOWN;
		break;

	case VK_LEFT:
		convertedKey = (char)GK_ARROW_LEFT;
		break;

	case VK_RIGHT:
		convertedKey = (char)GK_ARROW_RIGHT;
		break;

	case VK_SHIFT:
		convertedKey = (char)GK_SHIFT;
		isShiftDown = TRUE;
		break;

	case VK_CONTROL:
		convertedKey = (char)GK_CONTROL;
		break;
	}

	if (convertedKey == NULL)
	{
		if (isShiftDown)
		{
			switch (wParam)
			{
			case '`':
				convertedKey = '~';
				break;

			case '1':
				convertedKey = '!';
				break;

			case '2':
				convertedKey = '@';
				break;

			case '3':
				convertedKey = '#';
				break;

			case '4':
				convertedKey = '$';
				break;

			case '5':
				convertedKey = '%';
				break;

			case '6':
				convertedKey = '^';
				break;

			case '7':
				convertedKey = '&';
				break;

			case '8':
				convertedKey = '*';
				break;

			case '9':
				convertedKey = '(';
				break;

			case '0':
				convertedKey = ')';
				break;

			case '-':
				convertedKey = '_';
				break;

			case '=':
				convertedKey = '+';
				break;

			case '[':
				convertedKey = '{';
				break;

			case ']':
				convertedKey = '}';
				break;

			case '\\':
				convertedKey = '|';
				break;

			case ';':
				convertedKey = ':';
				break;

			case '\'':
				convertedKey = '"';
				break;

			case ',':
				convertedKey = '<';
				break;

			case '.':
				convertedKey = '>';
				break;

			case '/':
				convertedKey = '?';
				break;

			default:
				convertedKey = wParam;
				break;
			}
		}

		else if (wParam > 64 && wParam < 91)
			convertedKey = wParam + 32;

		else
			convertedKey = wParam;
	}

	// add the key to buffer
	AddToBuffer(convertedKey);

	if (isSendingInterrupts)
		((DCPU16*)dcpuPtr)->TriggerInterrupt(convertedKey);

	return TRUE;
}

BOOL GenericKeyboard::KeyUp(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0x10)
		isShiftDown = FALSE;

	return TRUE;
}

BOOL GenericKeyboard::AddToBuffer(char toAdd)
{
	if (bPosition == 16)
		bPosition = 0;

	buffer[bPosition] = toAdd;
	bPosition++;

	return (bPosition != 0);
}

#endif
