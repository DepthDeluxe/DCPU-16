#ifndef CONDITIONS_H
#define CONDITIONS_H

#include <string>
using namespace std;

#include "Dictionary.h"

#define CONDITION_FAIL -1
#define CONDITION_LABEL -2

struct ConditionReturn
{
	int value;
	int nextword;
	BOOL isLabel;

	ConditionReturn() { value=-1; nextword=-1; isLabel=FALSE; }
};

ConditionReturn TryRegValue(string text, Dictionary& dictionary);
ConditionReturn TryRawInt(string text);
ConditionReturn TryAddBy(string text, Dictionary& dictionary);
ConditionReturn	TryLabel(string text, Dictionary& valuesDict, Dictionary& labelDict);

#endif