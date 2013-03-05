#ifndef DCPUHARDWARE_H
#define DCPUHARDWARE_H

#include <windows.h>

// container class for all dcpu hardware

class DCPUHardware
{
public:
	DCPUHardware(void* cpuPtr);

	// holds pointer to interrupt function
	virtual void InterruptHandler(UINT16* DCPURegisters) { }
	void* dcpuPtr;

protected:
	// all requirements
	UINT	hardwareId;
	UINT16	hardwareVersion;
	UINT	manufacturer;

	UINT16	interruptMessage;
};

#endif
