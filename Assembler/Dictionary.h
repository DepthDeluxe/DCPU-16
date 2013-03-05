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

	UINT Length();
	
	int& operator[] (const char* key);		// bracket, [], override
};

#endif