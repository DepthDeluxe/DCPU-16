//	File: InterruptQueue.h
//
//	Author: Colin Heinzmann
//	Description: This is a class that creates an interrupt queue for use in the DCPU

#ifndef INTERRUPT_QUEUE_H
#define INTERRUPT_QUEUE_H

#include <windows.h>

#define INTERRUPT_QUEUE_MAX_SIZE	256

class InterruptQueue
{
	// members
private:
	UINT head;
	UINT count;
	UINT16 messages[INTERRUPT_QUEUE_MAX_SIZE];

public:
	// constructor
	InterruptQueue();

public:
	// standard queue functions
	BOOL EnQueue(UINT16 item);
	UINT16 DeQueue();

	// accessor functions
public:
	BOOL isEmpty();
	UINT Length();
};

#endif
