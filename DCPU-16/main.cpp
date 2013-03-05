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
	// create devices
	DCPU16 dcpu;

	M35FD floppyDrive(dcpu.GetRamPtr(), &dcpu);
	GenericKeyboard keyboard(&dcpu);
	LEM1802 display(dcpu.GetRamPtr(), &keyboard, &dcpu);

	// connect the devices to dcpu
	dcpu.ConnectHardware(&floppyDrive);		// dev 0
	dcpu.ConnectHardware(&keyboard);		// dev 1
	dcpu.ConnectHardware(&display);			// dev 2

	string input;

	cout << "Enter name of floppy drive file (boot.dflop assumed if left blank)" << endl;
	cout << "-> ";

	//getline(cin, input);

	if (input.length() == 0)
		input = "boot.dflop";

	floppyDrive.LoadDrive((char*)input.c_str());

	//todo: load the BIOS into the system memory so the thing can boot itself up
	{
		ifstream inFile;
		inFile.open("code.dexe", ios::binary);

		if (!inFile.is_open())
		{
			cout << "Error: Couldn't find bios!" << endl;
			return;
		}

		// get file length
		inFile.seekg(0, ios::end);
		int fileLen = inFile.tellg();
		inFile.seekg(0, ios::beg);

		// get the code
		UINT16* biosCode = new UINT16[fileLen/2];
		inFile.read((char*)biosCode, fileLen);

		// and set to cpu ram
		dcpu.SetRam(0, fileLen/2, biosCode);
	}

	// perform instructions until the thing turns off
	while (true)
		dcpu.NextInstruction();
}