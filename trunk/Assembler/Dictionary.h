//	File: Dictionary.h
//
//	Author: Colin Heinzmann
//	Description: Basic dictionary class that makes mapping the text instructions
//				 to their binary values making life easier)

#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <windows.h>
#include <vector>
#include <string>
using namespace std;

#define DICTIONARY_INVALID_KEY	500

class Dictionary
{
private:
	vector<string>	keys;
	vector<int>		items;

	UINT length;

public:
	Dictionary();		// constructor

	BOOL AddItem(int item, const char* key);
	BOOL RemoveItem(int item, const char* key);

	int GetKeyNumber(const char* key);

	UINT Length();

	BOOL IsAKey(string key);
	BOOL IsAKey(const char* key);
	
	int& operator[] (const char* key);		// bracket, [], override
	int& operator[] (int number);
};

#endif
