#ifndef MAIN_CPP
#define MAIN_CPP

// external includes
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
using namespace std;

// personal includes
#include "dcpuspecs.h"
#include "Dictionary.h"
#include "conditions.h"

// helper functions
vector<string> SplitString(string toSplit, char delim);
ConditionReturn ProcessValue(string text, Dictionary& dict);

// functions that load dictionaries
void SetUpOpcodes(Dictionary* dictionary);
void SetUpValues(Dictionary* dictionary);

// data structure to hold the information about 
struct AssemblyLabel
{
	string name;
	UINT16 pointer;

	// make sure value is set to zero on init
	AssemblyLabel()
	{
		pointer = 0;
	}

	AssemblyLabel(string s)
	{
		name = s;
		pointer = 0;
	}
};

struct ReferencePointer
{
	int instructionAddress;		// holds the position of the instruction in memory
	int offsetValue;			// holds the value that the label reference should be offset by
	int labelNumber;				// holds the label number

	ReferencePointer()
	{
		instructionAddress = 0;
		offsetValue = 0;
		labelNumber = 0;
	}

	ReferencePointer(int pointer, int offset, int label)
	{
		instructionAddress = pointer;
		offsetValue = offset;
		labelNumber = label;
	}
};

void main(int argc, char** argv)
{
	if (argc == 1)
	{
		cout << "=====================================" << endl;
		cout << "DCPU-16 Assembler v1.0" << endl;
		cout << "	Type file name after program to run" << endl;
		cout << "=====================================" << endl << endl;
		return;
	}

	if (argc != 2)
	{
		cout << "Error: Invalid number of arguments!" << endl << endl;
		return;
	}

	ifstream inFile;
	inFile.open(argv[1]);

	if (!inFile.is_open())
	{
		cout << "Error: Can't open file!" << endl << endl;
		return;
	}

	// read in all line information
	vector<string> fileContents;
	while (!inFile.eof())
	{
		string lineContent;
		getline(inFile,lineContent);

		fileContents.push_back(lineContent);
	}

	// entire file is now in memory
	inFile.close();

	// save all labels in program
	vector<AssemblyLabel> labels;
	for(UINT n = 0; n < fileContents.size(); n++)
	{
		if (fileContents[n].size() == 0)
			continue;

		if (fileContents[n][0] == ':')
		{
			// get rid of : anything after the first word
			string labelName = SplitString(fileContents[n], ' ')[0];
			labelName = &labelName[1];

			AssemblyLabel tempLabel(labelName);

			// check to see if there is already another label of the same name
			bool canAdd = true;
			for (UINT n = 0; n < labels.size(); n++)
			{
				if (labels[n].name == tempLabel.name)
				{
					cout << "Error: Label \"" << labels[n].name << "\" has been previously used!" << endl << endl;
					return;
				}
			}

			// add to label list
			if (tempLabel.name.length() > 0)
				labels.push_back(tempLabel);
		}
	}


	vector<UINT16> assembledCode;				// container in memory for assembled code
	UINT16 instructionCount = 0;				// keeps track of the number of instructions
	vector<ReferencePointer> labelReferences;	// This holds information about which pieces of code have given references
												// to labels.  After putting all instructions together, the program will
												// need to go back and put the proper pointer addresses in

	// load the opcode dictionary
	Dictionary opCodeDict;
	SetUpOpcodes(&opCodeDict);

	// load the values dictionary
	Dictionary valuesDictionary;
	SetUpValues(&valuesDictionary);

	for (UINT lineNumber = 0; lineNumber < fileContents.size(); lineNumber++)
	{
		// don't do anything if there is nothing on the line
		if (fileContents[lineNumber].length() == 0)
			continue;

		string lineText = fileContents[lineNumber];

		// skip line if it starts with comments
		if (lineText[0] == ';')
			continue;

		// save instruction pointer at that location
		if (lineText[0] == ':')
		{
			string labelName = SplitString(lineText, ' ')[0];
			labelName = &labelName[1];

			// search for label and change the pointer
			for (UINT n = 0; n < labels.size(); n++)
			{
				if (labels[n].name == labelName)
				{
					labels[n].pointer = instructionCount;
					break;
				}
			}

			// remove the first part of the line and
			// perform normal operation on it only
			// if the line text keeps going on
			UINT offset = SplitString(lineText, ' ')[0].length();

			if (offset < lineText.length())
				lineText = &lineText[offset];
			else
				continue;
		}

		// get rid of in-line comments
		vector<string> lineComponents = SplitString(lineText, ';');
		lineText = lineComponents[0];

		lineComponents = SplitString(lineText, ' ');

		// look at the first line entry for the command
		string command = lineComponents[0];

		// get a and b arguments
		string aText,bText = "null";

		switch(lineComponents.size())
		{
			// normal command
		case 3:
			aText = lineComponents[2];				// it is the second entry
			bText = lineComponents[1];				// it is the first entry
			bText.resize(bText.length() - 1);		// remove the comma after
			break;

			// special command
		case 2:
			aText = lineComponents[1];
			break;

		default:
			cout << "Error: Line " << lineNumber+1 << " has too many arguments!" << endl << endl;
			return;
		}
		
		// binary parts of instruction
		int binaryCommand;

		//
		// Get the binary command
		//
		try
		{
			// try to set the command
			binaryCommand = opCodeDict[command.c_str()];
		}
		catch(int e)
		{
			// command not found in dict
			cout << "Error: Line " << lineNumber+1 << " has an invalid command!" << endl;
			return;
		}

		// do other stuff if the command is DAT
		if (binaryCommand == DAT)
		{
			// get rid of starting DAT
			lineText = &lineText[4];

			// do other stuff
		}

		// get the A and B values
		ConditionReturn aReturn, bReturn;
		aReturn = ProcessValue(aText, valuesDictionary);

		// only process b command if bText has been set
		if (bText != "null")
			bReturn = ProcessValue(bText, valuesDictionary);
		else
		{
			// if it is a special opcode, command should be put in b value
			bReturn.value = binaryCommand;
			binaryCommand = 0;
		}

		// now combine all into one instruction
		UINT16 instruction = 0;

		instruction += aReturn.value, 6;
		instruction = instruction << 5;

		instruction += bReturn.value, 5;
		instruction = instruction << 5;

		instruction += binaryCommand;

		// now add it to binary saved in code
		assembledCode.push_back(instruction);
		instructionCount++;

		if (aReturn.nextword != -1)
		{
			assembledCode.push_back(aReturn.nextword);
			instructionCount++;
		}
		if (bReturn.nextword != -1)
		{
			assembledCode.push_back(bReturn.nextword);
			instructionCount++;
		}
	}

	// once done compiling everything, fix label references
	for (UINT n = 0; n < labelReferences.size(); n++)
	{
		int wordToBeChanged = labelReferences[n].instructionAddress;
		int labelNumber = labelReferences[n].labelNumber;

		assembledCode[wordToBeChanged] = labels[labelNumber].pointer;
	}

	// now save all program information
	string outFileName = SplitString(string(argv[1]), '.')[0];
	outFileName += string(".dexe");

	ofstream outFile;
	outFile.open(outFileName, ios::binary);

	// save the program to a file :D
	for (UINT n = 0; n < assembledCode.size(); n++)
		outFile.write((char*)&assembledCode[n], 2);

	outFile.close();
}

//
// this function is used to split up features in line
//
vector<string> SplitString(string toSplit, char delim)
{
	int len = toSplit.length();
	int startChar = 0;
	string split;

	vector<string> out;

	for (int n = 0; n < len; n++)
	{
		if (toSplit[n] == delim)
		{
			split = &toSplit[startChar];
			split.resize(n - startChar);

			out.push_back(split);

			startChar = n+1;
		}
	}

	// get the last bit
	if (startChar < len)
	{
		split = &toSplit[startChar];
		out.push_back(split);
	}

	return out;
}

// this function processes all of the possible options for a value to have
ConditionReturn ProcessValue(string text, Dictionary& dict)
{
	ConditionReturn result;

	result = TryRegValue(text, dict);
	if (result.value != -1)
		return result;

	result = TryRawInt(text);
	if (result.value != -1)
		return result;

	result = TryAddBy(text, dict);
	if (result.value != -1)
		return result;

	result.value = -1;
	return result;
}

void SetUpOpcodes(Dictionary* dictionary)
{
	//
	// add all the values to the dictionary
	//

	// normal opcodes
	dictionary->AddItem(SET, "SET");
	dictionary->AddItem(ADD, "ADD");
	dictionary->AddItem(SUB, "SUB");
	dictionary->AddItem(MUL, "MUL");
	dictionary->AddItem(MLI, "MLI");
	dictionary->AddItem(DIV, "DIV");
	dictionary->AddItem(DVI, "DVI");
	dictionary->AddItem(MOD, "MOD");
	dictionary->AddItem(MDI, "MDI");
	dictionary->AddItem(AND, "AND");
	dictionary->AddItem(BOR, "BOR");
	dictionary->AddItem(XOR, "XOR");
	dictionary->AddItem(SHR, "SHR");
	dictionary->AddItem(ASR, "ASR");
	dictionary->AddItem(SHL, "SHL");
	dictionary->AddItem(IFB, "IFB");
	dictionary->AddItem(IFC, "IFC");
	dictionary->AddItem(IFE, "IFE");
	dictionary->AddItem(IFN, "IFN");
	dictionary->AddItem(IFG, "IFG");
	dictionary->AddItem(IFA, "IFA");
	dictionary->AddItem(IFL, "IFL");
	dictionary->AddItem(IFU, "IFU");
	dictionary->AddItem(ADX, "ADX");
	dictionary->AddItem(SBX, "SBX");
	dictionary->AddItem(STI, "STI");
	dictionary->AddItem(STD, "STD");
	
	// special opcodes
	dictionary->AddItem(JSR, "JSR");
	dictionary->AddItem(SINT, "SINT");
	dictionary->AddItem(IAG, "IAG");
	dictionary->AddItem(RFI, "RFI");
	dictionary->AddItem(IAQ, "IAQ");
	dictionary->AddItem(HWN, "HWN");
	dictionary->AddItem(HWQ, "HWQ");
	dictionary->AddItem(HWI, "HWI");

	// data command
	dictionary->AddItem(DAT, "DAT");
}

void SetUpValues(Dictionary* dictionary)
{
	// register values
	dictionary->AddItem(REG_A, "A");
	dictionary->AddItem(REG_B, "B");
	dictionary->AddItem(REG_C, "C");
	dictionary->AddItem(REG_X, "D");
	dictionary->AddItem(REG_Y, "E");
	dictionary->AddItem(REG_Z, "F");
	dictionary->AddItem(REG_I, "G");
	dictionary->AddItem(REG_J, "H");
	
	// special values
	dictionary->AddItem(PUSH, "PUSH");
	dictionary->AddItem(POP, "POP");
	dictionary->AddItem(PEEK, "[SP]");
	dictionary->AddItem(SP, "SP");
	dictionary->AddItem(PC, "PC");
	dictionary->AddItem(EX, "EX");
}

#endif
