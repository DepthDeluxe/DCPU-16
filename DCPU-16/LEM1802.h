#ifndef LEM1802_H
#define LEM1802_H

// include windows files
#include <windows.h>
#include <d3d9.h>

#include "DCPUHardware.h"
#include "GenericKeyboard.h"
#include "Pixel.h"

// add the d3d9 library file and windows shtuff to project
#pragma comment(lib, "user32.lib")		// for the display window
#pragma comment(lib, "d3d9.lib")		// for using directx

// this is the same aspect ratio as the
// emulated screen
#define	D3D_SCREEN_WIDTH	640
#define D3D_SCREEN_HEIGHT	480
#define CUSTOMFVF			(D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

// this is a 128x96 pixel display
#define LEM1802_WIDTH		128
#define LEM1802_HEIGHT		96
#define DISPLAY_WIDTH		LEM1802_WIDTH
#define DISPLAY_HEIGHT		LEM1802_HEIGHT
#define TEXT_WIDTH			32
#define TEXT_HEIGHT			12

// interrupt commands
#define MEM_MAP_SCREEN		0
#define MEM_MAP_FONT		1
#define MEM_MAP_PALETTE		2
#define SET_BORDER_COLOR	3
#define MEM_DUMP_FONT		4
#define MEM_DUMP_PALETTE	5

// input for the separate thread (SWT stands for separate worker thread)
struct SWT_INPUT
{
	void* classPtr;
	GenericKeyboard* keyboardPtr;
	HWND* hWndPtr;
	BOOL* initialized;
	BOOL* threadKillFlag;
	BOOL* isThreadKilled;
};

// custom vertex structure for d3d
#ifndef CUSTOMVERTEX_CLASS
#define CUSTOMVERTEX_CLASS
struct CUSTOMVERTEX
{
	FLOAT x, y, z, rhw;
	DWORD color;
};
#endif

class LEM1802 : public DCPUHardware
{
private:
	UINT16* DCPURamPtr;		// points to the start of the video ram
	UINT16*	videoRam;		// pointer to starting location of video ram
	UINT16	Palette[16];	// this display contains a built-in 16 color palette
	UINT16	FontRam[256];	// contains 128 font entries

	// pixel handler class (there are 12288 pixels in a 128x96 display)
	Pixel	pixels[12288];

	// thread management objects
	BOOL	threadKillFlag;
	BOOL	isThreadKilled;

	// Direct3D objects
	LPDIRECT3D9				d3d;			// direct3d main object
	LPDIRECT3DDEVICE9		d3ddev;			// the device
	LPDIRECT3DVERTEXBUFFER9	vertexBuffer;	// vertex buffer

public:
	LEM1802(UINT16* ramPtr, GenericKeyboard* keyboard, void* cpuPtr);		// default constructor, will init the separate worker thread
	void InitDisplay(HWND hWnd);	// initializes all resources necessary for a working virtual display

	virtual void InterruptHandler(UINT16* DCPURegisters);
	void Interrupt(UINT16 command, UINT16 value);		// interrupt handling function
	BOOL IsConnected();									// checks to see if display window is up by looking at d3d status

	void Draw();			// draws stuff to screen

	void CleanD3D();		// cleans d3d interface
	~LEM1802();
};

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void SeperateWindowThreadFunc(SWT_INPUT* in);

#endif
