#pragma once
#include <math.h>

typedef struct _State {
	float x;
	float v;
} _State;

typedef struct Derivative {
	float dx;
	float dv;
} Derivative;

float acceleration(const _State &state, float t);
Derivative evaluate(const _State &initial, float t);
Derivative evaluate(const _State &initial, float t, float dt, const Derivative &d);
void integrate(_State &state, float t, float dt,int flag);
_State interpolate(const _State &previous, const _State &current, float alpha);