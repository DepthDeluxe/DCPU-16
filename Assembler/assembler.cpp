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

#define INVALID_LABEL_POS	-1

// helper functions
vector<string> SplitString(string toSplit, char delim);
ConditionReturn ProcessValue(string text, Dictionary& valuesDict, Dictionary& labelsDict);

// functions that load dictionaries
void SetUpOpcodes(Dictionary* dictionary);
void SetUpValues(Dictionary* dictionary);

struct ReferencePointer
{
	int instructionNumber;
	int labelNumber;
	BOOL refersToA;

	ReferencePointer(int instructionNum, int labelNum, BOOL isAReference)
	{
		instructionNumber = instructionNum;
		labelNumber = labelNum;
		refersToA = isAReference;
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

	// get all the labels for easy label checking later on
	Dictionary labelsDictionary;
	for(UINT n = 0; n < fileContents.size(); n++)
	{
		if (fileContents[n].size() == 0)
			continue;

		if (fileContents[n][0] == ':')
		{
			// get rid of : anything after the first word
			string labelName = SplitString(fileContents[n], ' ')[0];
			labelName = &labelName[1];

			// check to see if this label already exists
			if (labelsDictionary.IsAKey(labelName))
			{
				cout << "Error: Line " << n << " has a label redefinition!" << endl;
				return;
			}

			// add the label with a null value (will be replaced later)
			labelsDictionary.AddItem(NULL, labelName.c_str());
		}
	}


	vector<DCPU_Instruction> instructions;		// holds the instructions before complete construction
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

		// save the instruction count at the label point
		if (lineText[0] == ':')
		{
			string labelName = SplitString(lineText, ' ')[0];
			labelName = &labelName[1];

			// look for a label and change the pointer
			try
			{
				labelsDictionary[labelName.c_str()] = instructionCount;
			}
			catch(int e)
			{
				cout << "Something weird happened with the labels..." << endl;
				return;
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
			if (command != "DAT")
			{
				cout << "Error: Line " << lineNumber+1 << " has too many arguments!" << endl << endl;
				return;
			}
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
			if (command == "DAT")
			{
				// return value is crap
				BOOL result = ProcessDat(&lineText[4], instructions, instructionCount);
				
				// if DAT didn't work, throw an error
				if (!result)
				{
					cout << "Error: Line " << lineNumber+1 << " has invalid DAT values!" << endl;
					return;
				}

				// skip to the next line
				continue;
			}

			// command not found in dict
			cout << "Error: Line " << lineNumber+1 << " has an invalid command!" << endl;
			return;
		}

		// get the A and B values
		ConditionReturn aReturn, bReturn;
		aReturn = ProcessValue(aText, valuesDictionary, labelsDictionary);
		if (aReturn.isLabel)
			labelReferences.push_back( ReferencePointer(instructions.size(), aReturn.nextword, TRUE) );

		// only process b command if bText has been set
		if (bText != "null")
		{
			bReturn = ProcessValue(bText, valuesDictionary, labelsDictionary);
			if (bReturn.isLabel)
				labelReferences.push_back( ReferencePointer(instructions.size(), bReturn.nextword, FALSE) );
		}
		else
		{
			// if it is a special opcode, command should be put in b value
			bReturn.value = binaryCommand;
			binaryCommand = 0;
		}

		// assemble the instruction and add to the list
		DCPU_Instruction instruction;
		instruction.opcode	= binaryCommand;
		instruction.a		= aReturn.value;
		instruction.b		= bReturn.value;
		instruction.nextA	= aReturn.nextword;
		instruction.nextB	= bReturn.nextword;
		instruction.isALabel = aReturn.isLabel;
		instruction.isBLabel = bReturn.isLabel;

		instructions.push_back(instruction);

		// increment the instruction count
		instructionCount += 1 + (int)(instruction.nextA != -1) + (int)(instruction.nextB != -1);
	}

	// now fix all the label references so that they reflect the appropriate position
	for (UINT n = 0; n < labelReferences.size(); n++)
	{
		ReferencePointer pointer = labelReferences[n];
		DCPU_Instruction* instruction = &instructions[pointer.instructionNumber];

		if (pointer.refersToA)
			instruction->nextA = labelsDictionary[pointer.labelNumber];
		else
			instruction->nextB = labelsDictionary[pointer.labelNumber];
	}

	// now save all program information
	string outFileName = SplitString(string(argv[1]), '.')[0];
	outFileName += string(".dexe");

	ofstream outFile;
	outFile.open(outFileName, ios::binary);

	// now write the instructions to the file
	for(UINT n = 0; n < instructions.size(); n++)
	{
		// get an instruction
		DCPU_Instruction instruction = instructions[n];
		UINT16 binary = 0;

		// check for NO_OP
		if (instruction.opcode != NO_OP_DATA)
		{
			// convert it to binary form
			binary += instruction.a;
			binary = binary << 5;

			binary += instruction.b;
			binary = binary << 5;

			binary += instruction.opcode;
		}
		else
		{
			binary = instruction.a;
		}

		// write the binary to the file
		outFile.write((char*)&binary, 2);

		// add the extra bytes on the end if necessary
		if (instruction.nextA != -1)
		{
			UINT16 nextA = (UINT16)instruction.nextA;
			outFile.write((char*)&nextA, 2);
		}
		if (instruction.nextB != -1)
		{
			UINT16 nextB = (UINT16)instruction.nextB;
			outFile.write((char*)&nextB, 2);
		}
	}

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
ConditionReturn ProcessValue(string text, Dictionary& valuesDict, Dictionary& labelsDict)
{
	ConditionReturn result;

	result = TryRegValue(text, valuesDict);
	if (result.value != -1)
		return result;

	result = TryRawInt(text);
	if (result.value != -1)
		return result;

	result = TryAddBy(text, valuesDict);
	if (result.value != -1)
		return result;

	result = TryLabel(text, valuesDict, labelsDict);
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
