#ifndef CONDITIONS_H
#define CONDITIONS_H

#include <string>
using namespace std;

#include "Dictionary.h"

#define CONDITION_FAIL -1

struct ConditionReturn
{
	int value;
	int nextword;

	ConditionReturn() { value=-1; nextword=-1; }
};

ConditionReturn TryRegValue(string text, Dictionary& dictionary);
ConditionReturn TryRawInt(string text);
ConditionReturn TryAddBy(string text, Dictionary& dictionary);


#endif