#ifndef PIXEL_CPP
#define PIXEL_CPP

#include "Pixel.h"

Pixel::Pixel()
{
	x = 0;
	y = 0;

	// set to 0 size
	Move(0,0);
	Resize(0,0);
	SetColor(0,0,0);
}

Pixel::Pixel(UINT xPos, UINT yPos, UINT pWidth, UINT pHeight)
{
	x = 0;
	y = 0;

	Move(xPos, yPos);
	Resize(pWidth, pHeight);

	// set pixel to white
	SetColor(255, 255, 255);
}

void Pixel::Move(UINT destX, UINT destY)
{
	x = destX;
	y = destY;

	// top left
	vertices[0].x = x;
	vertices[0].y = y;
	vertices[0].z = 0;
	vertices[0].rhw = 0;

	// top right
	vertices[1].x = x + width;
	vertices[1].y = y;
	vertices[1].z = 0;
	vertices[1].rhw = 0;

	// bottom left
	vertices[2].x = x;
	vertices[2].y = y + height;
	vertices[2].z = 0;
	vertices[2].rhw = 0;

	// bottom left 2
	vertices[3].x = x;
	vertices[3].y = y + height;
	vertices[3].z = 0;
	vertices[3].rhw = 0;

	// top right 2
	vertices[4].x = x + width;
	vertices[4].y = y;
	vertices[4].z = 0;
	vertices[4].rhw = 0;

	// bottom right
	vertices[5].x = x + width;
	vertices[5].y = y + height;
	vertices[5].z = 0;
	vertices[5].rhw = 0;
}

void Pixel::Resize(UINT pWidth, UINT pHeight)
{
	width = pWidth;
	height = pHeight;

	// run move command to get all the vertices layed out
	Move(x,y);
}

void Pixel::SetColor(UINT r, UINT g, UINT b)
{
	color = D3DCOLOR_XRGB(r,g,b);

	for (int n = 0; n < 6; n++)
		vertices[n].color = color;
}

void Pixel::SetColor(DWORD sColor)
{
	color = sColor;

	for (int n = 0; n < 6; n++)
		vertices[n].color = color;
}

UINT Pixel::GetX()
{
	return x;
}

UINT Pixel::GetY()
{
	return y;
}

UINT Pixel::GetWidth()
{
	return width;
}

UINT Pixel::GetHeight()
{
	return height;
}

DWORD Pixel::GetColor()
{
	return color;
}

CUSTOMVERTEX* Pixel::GetVertices()
{
	return vertices;
}

#endif
