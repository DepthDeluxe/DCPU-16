#ifndef INTERRUPT_QUEUE_CPP
#define INTERRUPT_QUEUE_CPP

#include "InterruptQueue.h"

InterruptQueue::InterruptQueue()
{
	head = 0;
	count = 0;
}

BOOL InterruptQueue::EnQueue(UINT16 item)
{
	// fail if the queue has gotten too large
	if (count == INTERRUPT_QUEUE_MAX_SIZE)
		return FALSE;

	// get the next array value past the tail
	int tail = (head + count) % INTERRUPT_QUEUE_MAX_SIZE;

	// save the message and increase the count
	messages[tail] = item;
	count++;

	return TRUE;
}

UINT16 InterruptQueue::DeQueue()
{
	// return -1 if the queue is empty
	if (isEmpty())
		return 0xffff;

	UINT16 returnValue = messages[head];
	
	// decrement the count and increment the head, if it has reached the end of the array, go back to start
	count--;
	head++;
	if (head == INTERRUPT_QUEUE_MAX_SIZE)
		head = 0;

	return returnValue;
}

BOOL InterruptQueue::isEmpty()
{
	return count == 0;
}

UINT InterruptQueue::Length()
{
	return count;
}

#endif
