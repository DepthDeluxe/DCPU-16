#include "DCPU16.h"
#include "M35FD.h"
#include "GenericKeyboard.h"
#include "LEM1802.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

void main()
{
	// initialize objects on heap
	DCPU16* dcpu				= new DCPU16();
	M35FD* floppyDrive			= new M35FD(dcpu->GetRamPtr(), dcpu);
	GenericKeyboard* keyboard	= new GenericKeyboard(dcpu);
	LEM1802* display			= new LEM1802(dcpu->GetRamPtr(), keyboard, dcpu);

	// connect the devices to dcpu
	dcpu->ConnectHardware(floppyDrive);		// dev 0
	dcpu->ConnectHardware(keyboard);		// dev 1
	dcpu->ConnectHardware(display);			// dev 2

	string input;

	cout << "Enter name of executable you wish to run (code.dexe assumed if left blank)" << endl;
	cout << "-> ";

	getline(cin, input);

	// note: instance of floppy drive has been created but won't be used for this demonstration
	if (input.length() == 0)
		input = "code.dexe";

	ifstream inFile;
	inFile.open(input.c_str(), ios::binary);

	if (!inFile.is_open())
	{
		cout << "Error: Couldn't find bios!" << endl;
		return;
	}

	// get file length
	inFile.seekg(0, ios::end);
	int fileLen = (int)inFile.tellg();
	inFile.seekg(0, ios::beg);

	// get the code
	UINT16* biosCode = new UINT16[fileLen/2];
	inFile.read((char*)biosCode, fileLen);

	// and set to cpu ram
	dcpu->SetRam(0, fileLen/2, biosCode);

	// perform instructions until the thing turns off
	while (true)
		dcpu->NextInstruction();
}
