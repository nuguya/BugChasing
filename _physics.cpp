#include "_physics.h"

float acceleration(const _State &state, float t)
{
	float u = 35.5;
	float b = 35.0f;
	return u * -9.8f;
}



Derivative evaluate(const _State &initial, float t)
{
	Derivative output;
	output.dx = initial.v;
	output.dv = acceleration(initial, t);
	return output;
}

Derivative evaluate(const _State &initial, float t, float dt, const Derivative &d)
{
	_State state;
	state.x = initial.x + d.dx*dt;
	state.v = initial.v + d.dv*dt;
	Derivative output;
	output.dx = state.v;
	output.dv = acceleration(state, t + dt);
	return output;
}

void integrate(_State &state, float t, float dt,int flag)
{
	Derivative a = evaluate(state, t);
	Derivative b = evaluate(state, t, dt*0.5f, a);
	Derivative c = evaluate(state, t, dt*0.5f, b);
	Derivative d = evaluate(state, t, dt, c);

	const float dxdt = 1.0f / 6.0f * (a.dx + 2.0f*(b.dx + c.dx) + d.dx);
	const float dvdt = 1.0f / 6.0f * (a.dv + 2.0f*(b.dv + c.dv) + d.dv);
	if (flag) {
		state.x = state.x + dxdt * dt;
		state.v = state.v + dvdt * dt;
	}
	else {
		state.x = state.x - dxdt * dt;
		state.v = state.v + dvdt * dt;
	}
}

_State interpolate(const _State &previous, const _State &current, float alpha)
{
	_State state;
	state.x = current.x*alpha + previous.x*(1 - alpha);
	state.v = current.v*alpha + previous.v*(1 - alpha);
	return state;
}