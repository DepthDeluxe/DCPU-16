#ifndef M35FD_CPP
#define M35FD_CPP

#include "M35FD.h"

void M35FD::Init(UINT16* ramPtr)
{
	hardwareId		= 0x4fd524c5;
	hardwareVersion	= 0x000b;
	manufacturer	= 0x1eb37e91;		// MACKAPAR

	DCPURamPtr = ramPtr;
	deviceState = STATE_NO_MEDIA;		// device starts with no media in it

	threadKillFlag = FALSE;	
	threadIsKilled = TRUE;		// the thread starts killed

	// create the separate thread
	DTR_PARAMS parameters;
	parameters.drive = this;
	parameters.threadKillFlag = &threadKillFlag;
	parameters.threadIsKilled = &threadIsKilled;
	DWORD threadId;
	threadHandle = CreateThread(
		NULL,
		64000,
		(LPTHREAD_START_ROUTINE)DiskThreadRoutine,
		&parameters,
		NULL,
		&threadId
		);

	// sit around until the other thread has been initialized
	while (threadIsKilled)
		Sleep(10);
}

M35FD::M35FD(UINT16* ramPtr, void* cpuPtr)
	: DCPUHardware(cpuPtr)
{
	this->Init(ramPtr);
}

M35FD::M35FD(UINT16* ramPtr, char* diskFileName, void* cpuPtr)
	: DCPUHardware(cpuPtr)
{
	this->Init(ramPtr);

	this->LoadDrive(diskFileName);
}

M35FD::~M35FD()
{
	// eject the drive
	this->EjectDrive();
}

void M35FD::InterruptHandler(UINT16* DCPURegisters)
{
	// get register A value
	UINT16* regA = &DCPURegisters[0];
	UINT16* regB = &DCPURegisters[1];
	UINT16* regC = &DCPURegisters[2];
	UINT16* regX = &DCPURegisters[3];
	UINT16* regY = &DCPURegisters[4];

	// set and put flags into registers
	switch (*regA)
	{
		// poll state
	case 0:
		*regB = deviceState;
		*regC = deviceError;
		deviceError = ERROR_NONE;
		break;

		// set interrupts - need to implement on hardware side
	case 1:
		break;

		// read
	case 2:
		if (deviceState == STATE_READY || deviceState == STATE_READY_WP)
		{
			buffer = &DCPURamPtr[*regX];	// set buffer location
			deviceState = STATE_BUSY;		// set busy flag which will activate the other thread
		}
		else if (!Disk.is_open())
		{
			deviceError = ERROR_NO_MEDIA;
		}
		else if (deviceState == STATE_BUSY)
		{
			deviceError = ERROR_BUSY;
		}
		break;

		// write
	case 3:
		if (deviceState == STATE_READY || deviceState == STATE_READY_WP)
		{
			buffer = &DCPURamPtr[*regX];	// set buffer location
			deviceState = STATE_BUSY;		// set busy flag which will activate the other thread
		}
		else if (!Disk.is_open())
		{
			deviceError = ERROR_NO_MEDIA;
		}
		else if (deviceState == STATE_BUSY)
		{
			deviceError = ERROR_BUSY;
		}
		else if (deviceState == STATE_READY_WP)
		{
			deviceError = ERROR_PROTECTED;
		}
		break;
	}
}

void M35FD::Run()
{
	// don't do anything if the state is not busy
	if (deviceState != STATE_BUSY)
		return;
}

BOOL M35FD::ReadSector(int sectorNumber)
{
	// return error and set error flag to broken if disk has been
	if (!Disk.is_open() || buffer == NULL)
	{
		deviceError = ERROR_BROKEN;
		return FALSE;
	}

	// seek to the beginning of the sector (512 2-byte words in sector)
	Disk.seekg(sectorNumber * 1024);

	// read 1 full sector into the buffer
	Disk.read((char*)buffer, 1024);

	return TRUE;
}

BOOL M35FD::WriteSector(int sectorNumber)
{
	// return error and set error flag to broken if disk has been
	if (!Disk.is_open() || buffer == NULL)
	{
		deviceError = ERROR_BROKEN;
		return FALSE;
	}

	// seek to the beginning of the sector (512 2-byte words in sector)
	Disk.seekg(sectorNumber * 1024);

	// write 1 full sector into the buffer
	Disk.write((char*)buffer, 1024);

	return TRUE;
}

BOOL M35FD::LoadDrive(char* filename)
{
	// don't auto eject if there is currently a file in there
	if (Disk.is_open())
		return FALSE;

	Disk.open(filename, ios::in | ios::out | ios::binary);

	if (!Disk.is_open())
		return FALSE;

	deviceState = STATE_READY;
	return TRUE;
}

BOOL M35FD::EjectDrive()
{
	// if there was no disk to begin with, just return OK
	if (!Disk.is_open())
		return TRUE;

	Disk.close();

	// the device is broken if something didn't work out
	if (Disk.is_open())
	{
		deviceError = ERROR_BROKEN;
		return FALSE;
	}

	deviceState = STATE_NO_MEDIA;
	return TRUE;
}

UINT16 M35FD::GetState()
{
	return deviceState;
}

void DiskThreadRoutine(DTR_PARAMS* params)
{
	M35FD* drive			= (M35FD*)params->drive;
	BOOL* threadKillFlag	= params->threadKillFlag;
	BOOL* threadIsKilled	= params->threadIsKilled;

	// now set the is killed flag to false so that everything else can resume
	*threadIsKilled = FALSE;

	while (!*threadKillFlag)
	{
		drive->Run();

		// sleep only if the device is currently not in use
		if (drive->GetState() != STATE_BUSY)
			Sleep(10);
	}

	*threadIsKilled = TRUE;
}

#endif
