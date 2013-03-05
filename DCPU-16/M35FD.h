#ifndef M35FD_H
#define M35FD_H

#include <windows.h>
#include <fstream>
using namespace std;

#include "DCPUHardware.h"

// state codes
#define STATE_NO_MEDIA	0x0000
#define STATE_READY		0x0001
#define STATE_READY_WP	0x0002
#define STATE_BUSY		0x0003

// define all the error codes for device
#define ERROR_NONE			0x0000
#define ERROR_BUSY			0x0001
#define ERROR_NO_MEDIA		0x0002
#define ERROR_PROTECTED		0x0003
#define ERROR_EJECT			0x0004
#define ERROR_BAD_SECTOR	0x0005
#define ERROR_BROKEN		0xffff

struct DTR_PARAMS
{
	void*	drive;
	BOOL*	threadKillFlag;
	BOOL*	threadIsKilled;
};

class M35FD : public DCPUHardware
{
private:
	fstream Disk;			// file stream used for reading and writing data

	UINT16*	DCPURamPtr;		// pointer to start of DCPU ram
	UINT16*	buffer;			// buffer to read/write location in ram
	
	UINT16	deviceState;	// saves the state of the device
	UINT16	deviceError;	// holds the device's latest error

	HANDLE	threadHandle;	// handle to the asynchronous thread
	BOOL	threadKillFlag; // flag to kill the worker thread
	BOOL	threadIsKilled;	// flag to tell main thread if thread has been killed

private:
	void Init(UINT16* ramPtr);

public:
	M35FD(UINT16* ramPtr, void* cpuPtr);
	M35FD(UINT16* ramPtr, char* diskFileName, void* cpuPtr);
	~M35FD();

	virtual void InterruptHandler(UINT16* DCPURegisters);	// handles hardware interrupts, as required by system
	void Run();												// provides ability to run everything inside another thread

	BOOL ReadSector(int sectorNumber);		// used to read a sector of drive
	BOOL WriteSector(int sectorNumber);		// used to write a sector of drive

	BOOL LoadDrive(char* filename);			// load the drive with file specified
	BOOL EjectDrive();						// eject the current drive

	UINT16	GetState();		// gets the state of the drive
};

void DiskThreadRoutine(DTR_PARAMS* params);

#endif
