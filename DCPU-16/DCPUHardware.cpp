#ifndef DCPUHARDWARE_CPP
#define DCPUHARDWARE_CPP

#include "DCPUHardware.h"

DCPUHardware::DCPUHardware(void* cpuPtr)
{
	dcpuPtr = cpuPtr;

	interruptMessage = NULL;
}

#endif
