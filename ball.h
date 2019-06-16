#pragma once
#include <stdlib.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

class Ball
{
public:
	D2D1_POINT_2F point;
	BOOL isselect;
	BOOL state;
public:
	Ball() {
		point.x = 350;
		point.y = 350;
		isselect = FALSE;
		state = FALSE;
	}
};