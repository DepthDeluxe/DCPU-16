#ifndef DICTIONARY_CPP
#define DICTIONARY_CPP

#include "Dictionary.h"

#include <iostream>

Dictionary::Dictionary()
{
	length = 0;
}

BOOL Dictionary::AddItem(int item, const char* key)
{
	keys.push_back(string(key));
	items.push_back(item);

	length++;

	return TRUE;
}

BOOL Dictionary::RemoveItem(int item, const char* key)
{
	for(UINT n = 0; n < length; n++)
	{
		// check to see if the string is the same
		if (strcmp(keys[n].c_str(), key) == 0)
		{
			// and remove
			keys.erase(keys.begin() + (int)n);
			items.erase(items.begin() + (int)n);

			return TRUE;
		}
	}

	return FALSE;
}

UINT Dictionary::Length()
{
	return length;
}

int& Dictionary::operator[](const char* key)
{
	for (UINT n = 0; n < length; n++)
	{
		if (strcmp(keys[n].c_str(), key) == 0)
		{
			return items[n];
		}
	}

	throw DICTIONARY_INVALID_KEY;		// throw an exception if the key wasn't found
}

#endif

#if 0

void main()
{
	string inString;
	Dictionary dict;

	while (true)
	{
		cout << "Key -> ";
		getline(cin, inString);
		cout << endl;

		string key = inString;

		cout << "Value -> ";
		getline(cin, inString);
		cout << endl;

		// add the item to the dictionary
		int value = atof(inString.c_str());
		dict.AddItem(value, key.c_str());

		cout << "What item? -> ";
		getline(cin, inString);

		if (dict[inString.c_str()] == 1)
			cout << "success"<< endl;

		else
			cout << "wrong -_-" << endl;
	}
}

#endif