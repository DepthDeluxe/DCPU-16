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
		result.value = 0x21;		// it is 0x21 because of the shifted literal def
		return result;
	}
	if (text == "[0]")
	{
		result.value = NEXTWORD_VALUE;
		result.nextword = 0;

		return result;
	}

	// first try raw value
	int value = atoi(text.c_str());

	// convert to literal value and return
	if (value != 0)
	{
		if (value > -2 && value < 31)
			result.value = value + 0x21;		// can put literal value in code
		else
		{
			result.value = NEXTWORD;
			result.nextword = value;
		}

		return result;
	}

	// don't do anything more if the string is less than 3 characters long
	if (text.length() < 3)
		return result;

	// now try to remove brackets (at start and end)
	text = &text[1];
	text.resize(text.length()-1);

	// and try the value again
	value = atof(text.c_str());

	if (value != 0)
	{
		result.value = NEXTWORD_VALUE;
		result.nextword = value;
	}

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
	if (splitLoc == text.length()-1)
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
	if (firstValue <= REG_J)
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

BOOL ProcessDat(string text, vector<DCPU_Instruction>& instructions, UINT16& instructionCount)
{
	bool inQuote = false;		// keeps track of the starting quote
	bool inLiteral = false;		// keeps track to see if the string is in a literal
	string tempStr = "";		// holds the temporary info
	for (int n = 0; n < text.length(); n++)
	{
		if (text[n] == ',' && !inQuote)
		{
			if (inLiteral)
			{
				int number;
				if (tempStr == "0")
					number = 0;
				else
				{
					number = atoi(tempStr.c_str());
					
					// fail if the string is invalid
					if (number == 0)
						return FALSE;
				}

				// add a new instruction with the NO_OP flag
				DCPU_Instruction newInstruction;
				newInstruction.opcode = NO_OP_DATA;
				newInstruction.a = number;
				newInstruction.nextA = -1;
				newInstruction.nextB = -1;

				instructions.push_back(newInstruction);
				instructionCount++;

				// clear the string and reset flag
				tempStr = "";
				inLiteral = false;
			}
			else
				inLiteral = true;

			// no matter what, move on
			continue;
		}

		if (text[n] == '\"' && !inLiteral)
		{
			if (inQuote)
			{
				// add all of the string values byte by byte
				for (int x = 0; x < tempStr.length(); x++)
				{
					DCPU_Instruction newInstruction;
					newInstruction.opcode = NO_OP_DATA;
					newInstruction.a = (int)tempStr[x];
					newInstruction.nextA = -1;
					newInstruction.nextB = -1;

					instructions.push_back(newInstruction);
					instructionCount++;
				}

				// reset the string and quote flag
				tempStr = "";
				inQuote = false;
			}
			else
				inQuote = true;

			// no matter what, continue
			continue;
		}

		// ignore spaces in literals
		if (text[n] == ' ' && !inQuote)
			continue;

		tempStr += text[n];
	}

	return TRUE;
}

#endif
