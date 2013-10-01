//	File: Pixel.h
//
//	Author: Colin Heinzmann
//	Description: Class that stores data to draw an individual pixel on the screen
//				 using the DirectX library

#ifndef PIXEL_H
#define PIXEL_H

#include <windows.h>
#include <d3d9.h>

// custom vertex structure for d3d
#ifndef CUSTOMVERTEX_CLASS
#define CUSTOMVERTEX_CLASS
struct CUSTOMVERTEX
{
	FLOAT x, y, z, rhw;
	DWORD color;
};
#endif

class Pixel
{
private:
	UINT x,y;
	UINT height,width;
	DWORD color;

	CUSTOMVERTEX vertices[6];

public:
	Pixel();
	Pixel(UINT xPos, UINT yPos, UINT pWidth, UINT pHeight);

	void Move(UINT destX, UINT destY);
	void Resize(UINT pWidth, UINT pHeight);
	void SetColor(UINT r, UINT g, UINT b);
	void SetColor(DWORD sColor);

	UINT GetX();
	UINT GetY();
	UINT GetHeight();
	UINT GetWidth();
	DWORD GetColor();
	CUSTOMVERTEX* GetVertices();
};

#endif
