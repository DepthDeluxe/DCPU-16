#ifndef LEM1802_CPP
#define LEM1802_CPP

#include "LEM1802.h"

#include <math.h>
#include <fstream>
#include <string>
#include <iostream>

using namespace std;

LEM1802::LEM1802(UINT16* ramPtr, GenericKeyboard* keyboard, void* cpuPtr)
	: DCPUHardware(cpuPtr)
{
	DCPURamPtr = ramPtr;

	// set hardware info
	hardwareId		= 0x7349f615;	// specific device hardware ID
	hardwareVersion	= 0x1802;		// LEM model 1802
	manufacturer	= 0x1c6c8b36;	// (NYA_ELEKTRISKA)

	// sets VideoRam pointer to null
	videoRam = NULL;

	// set D3D objects to null, these will tell the outside
	// world if LEM1802 has been shut down
	d3d				= NULL;
	d3ddev			= NULL;
	vertexBuffer	= NULL;

	// zero the palette and font ram
	memset(FontRam, 0, 512);
	memset(Palette, 0, 32);

	// create window objects
	HWND		hWnd		= NULL;
 	BOOL		initialized	= FALSE;

	// set the kill flag to false, when set to true
	threadKillFlag = FALSE;
	isThreadKilled = FALSE;

	SWT_INPUT	threadInput;
	threadInput.classPtr		= this;
	threadInput.keyboardPtr		= keyboard;
	threadInput.hWndPtr			= &hWnd;
	threadInput.initialized		= &initialized;
	threadInput.threadKillFlag	= &threadKillFlag;
	threadInput.isThreadKilled	= &isThreadKilled;

	// create the separate window thread
	DWORD threadId;
	HANDLE windowHandle = CreateThread(
		NULL,
		256000,
		(LPTHREAD_START_ROUTINE)SeperateWindowThreadFunc,
		&threadInput,
		NULL,
		&threadId
		);

	// wait until the window is initialized to continue doing stuff
	while (!initialized)
		Sleep(10);

	// load the default font cache from file
	ifstream inFile;
	inFile.open("default_font.ini");

	if (!inFile.is_open())
	{
		cout << "Error: default_font.ini missing!" << endl;
		this->~LEM1802();
		return;
	}

	string lineIn;
	for (int n = 0; n < 128 || !inFile.eof(); n++)
	{
		getline(inFile, lineIn);
		sscanf(lineIn.c_str(), "%x", &FontRam[n]);
	}

	// close the file
	inFile.close();

	// now init palette froom file
	inFile.open("default_pallete.ini");

	if (!inFile.is_open())
	{
		cout << "Error: default_pallete.ini missing!" << endl;
		this->~LEM1802();
		return;
	}

	// populate the palette with values from file in order that is listed in the file
	for (int n = 0; n < 16; n++)
	{
		getline(inFile, lineIn);

		sscanf(lineIn.c_str(), "%x", &Palette[n]);
	}

	inFile.close();
}

void LEM1802::InitDisplay(HWND hWnd)
{
	// start initializing Direct3D
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	// presentation parameters
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.hDeviceWindow = hWnd;

	// create device class using inputted information
	d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_SOFTWARE_VERTEXPROCESSING,
		&d3dpp,
		&d3ddev
		);

	// create resources necessary to make display
	// init the vertex buffer
	UINT vertexBufferLen = 6 * LEM1802_WIDTH * LEM1802_HEIGHT * sizeof(CUSTOMVERTEX);
	d3ddev->CreateVertexBuffer(
		vertexBufferLen,
		0,
		CUSTOMFVF,
		D3DPOOL_MANAGED,
		&vertexBuffer,
		NULL
		);

	UINT pixelWidth = D3D_SCREEN_WIDTH / LEM1802_WIDTH;
	UINT pixelHeight = D3D_SCREEN_HEIGHT / LEM1802_HEIGHT;

	for (int n = 0; n < DISPLAY_WIDTH * DISPLAY_HEIGHT; n++)
	{
		int x = n % DISPLAY_WIDTH;
		int y = floor((float)n / (float)DISPLAY_WIDTH);
		pixels[n].Move(x * pixelWidth, y * pixelHeight);
		pixels[n].Resize(pixelWidth, pixelHeight);
		pixels[n].SetColor(n % 256, n % 256, n % 256);	 // set to debugging color
	}
}

void LEM1802::InterruptHandler(UINT16* DCPURegisters)
{
	// grab register A and B values
	Interrupt(DCPURegisters[0], DCPURegisters[1]);
}

void LEM1802::Interrupt(UINT16 command, UINT16 value)
{
	switch(command)
	{
	case MEM_MAP_SCREEN:
		if (value != NULL)
			videoRam = &DCPURamPtr[value];
		else
			videoRam = NULL;
		break;
		
	case MEM_MAP_FONT:
		break;

	case MEM_MAP_PALETTE:
		break;

	case SET_BORDER_COLOR:
		break;

	case MEM_DUMP_FONT:
		break;

	case MEM_DUMP_PALETTE:
		break;
	}
}

//
// This function tests to see if the emulated window is open or not
//
BOOL LEM1802::IsConnected()
{
	return (BOOL)(d3d != NULL);
}

void LEM1802::Draw()
{
	// get the data from pointer only if video ram pointer is not null
	if (videoRam == 0)
		return;

	// process each character to display
	for (int n = 0; n < TEXT_WIDTH * TEXT_HEIGHT; n++)
	{
		// get each parameter out of the uint16
		UCHAR character = videoRam[n] & 0x7f;				// character selection (0-127)
		UCHAR isBlink = (videoRam[n] >> 7) & 0x01;			// blinking value	(0-1)
		UINT16 backColor = Palette[(videoRam[n] >> 8) & 0x0f];		// background color (0-15)
		UINT16 foreColor = Palette[(videoRam[n] >> 12) & 0x0f];		// foreground color (0-15)

		bool isOn[4][8];									// holds info about which pixel is on or off
		//UINT16* pixelMap = &FontRam[2 * character];			// holds the pixel map of desired character
															// this is 2 16-bit words or 1 32-bit word

		char* pixelMap = (char*)&FontRam[2 * character];

		for (int x = 0; x < 4; x++)
		{
			for (int y = 0; y < 8; y++)
			{
				isOn[x][y] = (pixelMap[(3-x) - 2*(x<2) + 2*(x>1)] >> y) & 0x1;		// need to remember that crap is saved in little endian mode
			}
		}

		// then map the appropriate color to the emulated pixels
		{
			// get foreground and background RGB values
			UINT foreR = 17 * ((foreColor >> 8) & 0xf);
			UINT foreG = 17 * ((foreColor >> 4) & 0xf);
			UINT foreB = 17 * (foreColor & 0xf);
			UINT backR = 17 * ((backColor >> 8) & 0xf);
			UINT backG = 17 * ((backColor >> 4) & 0xf);
			UINT backB = 17 * (backColor & 0xf);

			// these are starting pixels for mapping each character to the proper place on the emulated screen
			int textXPos = n % TEXT_WIDTH;
			int textYPos = floor((float)n / (float)TEXT_WIDTH);
			int displayXPos = textXPos * 4;
			int displayYPos = textYPos * 8;

			for (int x = 0; x < 4; x++)
			{
				for (int y = 0; y < 8; y++)
				{
					if (isOn[x][y])
						pixels[(x + displayXPos) + (y + displayYPos) * DISPLAY_WIDTH].SetColor(foreR, foreG, foreB);
					else
						pixels[(x + displayXPos) + (y + displayYPos) * DISPLAY_WIDTH].SetColor(backR, backG, backB);
				}
			}
		}
	}

	{
		// lock the vertex buffer for memory copying
		CUSTOMVERTEX* vertices;
		int numPixels = DISPLAY_WIDTH * DISPLAY_HEIGHT;
		vertexBuffer->Lock(0, 6 * numPixels * sizeof(CUSTOMVERTEX), (void**)&vertices, 0);

		// copy pixels into video memory
		for (int n = 0; n < numPixels; n++)
			memcpy(&vertices[6*n], pixels[n].GetVertices(), 6 * sizeof(CUSTOMVERTEX));

		// unlock buffer
		vertexBuffer->Unlock();
	}

	// clear and begin drawing the scene
	d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	d3ddev->BeginScene();

	// do rendering and such...
	d3ddev->SetFVF(CUSTOMFVF);
	d3ddev->SetStreamSource(0, vertexBuffer, 0, sizeof(CUSTOMVERTEX));
	d3ddev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 2 * LEM1802_WIDTH * LEM1802_HEIGHT);

	// end and present the rendering
	d3ddev->EndScene();
	d3ddev->Present(NULL, NULL, NULL, NULL);
}

void LEM1802::CleanD3D()
{
	// release the two d3d objects
	d3ddev->Release();
	d3d->Release();

	// and set to null
	d3ddev = NULL;
	d3d = NULL;
}

LEM1802::~LEM1802()
{
	// set the thread kill flag
	threadKillFlag = TRUE;

	// wait for the separate thread to be killed
	while( !isThreadKilled )
		Sleep(10);
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	GenericKeyboard* keyboard = (GenericKeyboard*)GetWindowLong(hWnd, GWL_USERDATA);

	switch(message)
	{
	case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		} break;

	case WM_KEYDOWN:
		keyboard->KeyDown(wParam, lParam);
		break;

	case WM_KEYUP:
		keyboard->KeyUp(wParam, lParam);
		break;
	}

	return DefWindowProc (hWnd, message, wParam, lParam);
}

void SeperateWindowThreadFunc(SWT_INPUT* in)
{
	LEM1802* display = (LEM1802*)in->classPtr;
	GenericKeyboard* keyboard = in->keyboardPtr;
	HWND* hWndPtr = in->hWndPtr;
	BOOL* initialized = in->initialized;
	BOOL* threadKillFlag = in->threadKillFlag;
	BOOL* isThreadKilled = in->isThreadKilled;
	HINSTANCE hInstance = GetModuleHandle(NULL);

	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));

	wcex.lpszClassName = L"EmulatedDisplay";
	wcex.cbSize = sizeof(wcex);
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(hInstance, IDC_ARROW);
	wcex.lpfnWndProc = WindowProc;

	if ( !RegisterClassEx(&wcex) )
		return;

	*hWndPtr = CreateWindowEx(
		NULL,
		L"EmulatedDisplay",
		L"LEM1802",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		D3D_SCREEN_WIDTH, D3D_SCREEN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL
		);

	if (!*hWndPtr)
	{
		*isThreadKilled = TRUE;
		return;
	}

	ShowWindow(*hWndPtr, SW_SHOWDEFAULT);
	SetWindowLong(*hWndPtr, GWL_USERDATA, (LONG)keyboard);	// send keyboard pointer to the window proc

	*initialized = TRUE;
	display->InitDisplay(*hWndPtr);

	MSG msg;
	while( !*threadKillFlag )
	{
		if (!GetMessage( &msg, 0, 0, 0 ))
			break;

		// before drawing, check to see if keys have been pressed
		//keyboard->Run();
		// ^^ from now on, the keyboard will be run from inside the terminal :D

		// perform rendering on another thread
		display->Draw();

		DispatchMessage(&msg);
	}

	// clean up the direct3d interface
	display->CleanD3D();

	*isThreadKilled = TRUE;
}

#endif
