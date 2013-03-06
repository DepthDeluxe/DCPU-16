#ifndef CONDITIONS_CPP
#define CONDITIONS_CPP

#include "conditions.h"

#include "dcpuspecs.h"

ConditionReturn TryRegValue(string text, Dictionary& dictionary)
{
	ConditionReturn result;

	// see if it works
	try
	{
		result.value = dictionary[text.c_str()];
	}
	catch (int e)
	{
		// don't do anything more if text is less than 3 characters
		if (text.length() < 3)
			return result;
		
		// if this didn't work, try removing the brackets
		text = &text[1];
		text.resize(text.length()-1);

		try
		{
			result.value = dictionary[text.c_str()];

			if (result.value != SP)
				result.value += 0x08;
			else
				result.value = 0x19;
		}
		catch (int e)
		{
			// if this didn't work, return -1, or false
			return result;
		}
	}

	// return the result
	return result;
}

ConditionReturn TryRawInt(string text)
{
	ConditionReturn result;

	if (text == "0")
	{
		result.value = 0;
		return result;
	}
	if (text == "[0]")
	{
		result.value = NEXTWORD_VALUE;
		result.nextword = 0;

		return result;
	}

	// first try raw value
	result.value = atof(text.c_str());

	// convert to literal value and return
	if (result.value != 0)
	{
		if (result.value > -2 && result.value < 31)
			result.value += 0x21;		// can put literal value in code
		else
		{
			result.nextword = result.value;
			result.value = NEXTWORD;
		}

		return result;
	}

	// don't do anything more if the string is less than 3 characters long
	if (text.length() < 3)
	{
		result.value = CONDITION_FAIL;
		return result;
	}

	// now try to remove brackets (at start and end)
	text = &text[1];
	text.resize(text.length()-1);

	// and try the value again
	result.value = atof(text.c_str());

	if (result.value != 0)
	{
		result.nextword = result.value;
		result.value = NEXTWORD_VALUE;
	}

	// if couldn't do anything, return false
	result.value = CONDITION_FAIL;
	return result;
}

ConditionReturn TryAddBy(string text, Dictionary& dictionary)
{
	ConditionReturn result;

	// first split up the string
	if (text.length() < 3 || text[0] != '[' || text[text.length()-1] != ']')
		return result;

	// remove the brackets
	text = &text[1];
	text.resize(text.length()-1);

	int splitLoc = -1;
	for (UINT n = 0; n < text.length(); n++)
	{
		splitLoc++;
		if (text[n] == '+')
			break;
	}

	// can't do anything if there was no plus
	if (splitLoc == text.length())
		return result;

	// split the two values
	string firstHalf = text;
	firstHalf.resize(splitLoc);
	string lastHalf = &text[splitLoc+1];

	// this will be some register value
	int firstValue = TryRegValue(firstHalf, dictionary).value;
	
	// return failure if this function failed
	if (firstValue == CONDITION_FAIL)
		return result;

	// try to convert it to a number
	int secondValue;
	if (lastHalf == "0")
		secondValue = 0;
	else
	{
		secondValue = atoi(lastHalf.c_str());

		// return failure if this didn't work
		if (secondValue == 0)
			return result;
	}

	// try to interpret the registers
	if (firstValue < REG_J)
	{
		result.value = firstValue + 0x10;
		result.nextword = secondValue;
	}

	else if (firstValue == SP)
	{
		result.value = PICK;
		result.nextword = secondValue;
	}

	return result;
}

ConditionReturn TryLabel(string text, Dictionary& valuesDict, Dictionary& labelDict)
{
	ConditionReturn result;

	// check for direct reference
	if (labelDict.IsAKey(text))
	{
		result.value = NEXTWORD;
		result.nextword = labelDict.GetKeyNumber(text.c_str());
		result.isLabel = TRUE;
		return result;
	}

	// try removing parenthesis
	if (text.length() < 3)
		return result;

	text = &text[1];
	text.resize(text.length() - 1);

	// check to see if it is a reference
	if (labelDict.IsAKey(text))
	{
		result.value = NEXTWORD_VALUE;
		result.nextword = labelDict.GetKeyNumber(text.c_str());
		result.isLabel = TRUE;
		return result;
	}
	
	// find where to split
	int splitLoc = -1;
	for (UINT n = 0; n < text.length(); n++)
	{
		if (text[n] == '+')
		{
			splitLoc = n;
			break;
		}
	}

	if (splitLoc == -1)
		return result;

	// and split
	string firstHalf = text;
	firstHalf.resize(splitLoc);
	string lastHalf = &text[splitLoc+1];

	int firstValue = TryRegValue(firstHalf, valuesDict).value;

	if (firstValue == CONDITION_FAIL)
		return result;

	if (labelDict.IsAKey(lastHalf))
	{
		if (firstValue == SP)
			result.value = PICK;
		else
			result.value = firstValue + 0x10;

		result.nextword = labelDict.GetKeyNumber(lastHalf.c_str());
		result.isLabel = TRUE;
	}

	return result;
}

#endif
