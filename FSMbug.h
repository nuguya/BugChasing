#pragma once
#pragma once
#include <d2d1.h>
#include "FiniteStateMachine.h"
#include "ball.h"
#include <stdlib.h>

// 평면의 크기. 좌우로 [-PLANE_SIZE,PLANE_SIZE], 앞뒤로도 [-PLANE_SIZE,PLANE_SIZE].
#define PLANE_SIZE_X 840
#define PLANE_SIZE_Y 640


class Character
{
public:
	enum StateID {
		STATE_UNDEFINED = 0,
		STATE_STAND,
		STATE_MOVE,
		STATE_RUNAWAY,
		STATE_FOLLOW,
		STATE_IDLE,
	};

	enum EventID {
		EVENT_UNDEFINED = 0,
		EVENT_FINDTARGET,
		EVENT_STOPWALK,
		EVENT_DUBIOUS,
		EVENT_LOSTTARGET,
		EVENT_DIE,
	};

	enum Type {
		TYPE_UNDEFINED = 0,
		TYPE_HUMAN,
		TYPE_AI,
	};
	enum Color { Black, Red, Blue, Green };
public:
	int id;
	int p;
	Type type;
	D2D1_POINT_2F position;
	D2D1_POINT_2F destPosition;
	D2D1_POINT_2F deadpoint;
	float rangeView; //볼 수 있는 범위.
	float rangeTouch; //닿을 수 있는 범위.
	float speed;
	float orientation;
	BOOL _isCatch=FALSE; //벌레가 잡혔는지 여부 확인
	float refangle=0.0f;
	FiniteStateMachine* stateMachine;
	Color color;//MeshMD2* model;

public:
	Character(Type _type, DWORD stateTransitions[][3], int numTransitions);//, MeshMD2* _model);
	~Character();

	void update(Ball* target, float timeDelta);

	void setPosition(D2D1_POINT_2F pos) { position = pos; }
	D2D1_POINT_2F getPosition() { return position; }
	float getRangeView() { return rangeView; }
	Color getColor() { return color; }//MeshMD2* getModel() { return model; }
	void setDestPosition(D2D1_POINT_2F dest);
	D2D1_POINT_2F getDestPositon() { return destPosition; }
	float getOrientation() { return orientation; }

private:
	void issueEvent(DWORD event);
	void moveTo(float timeDelta);
	bool isVisible(D2D1_POINT_2F pos);
	void updateHuman(float timeDelta);
	void updateAI(Ball* target, float timeDelta);

	void actionStand(float timeDelta);
	void actionMove(float timeDelta);
	void actionFollow(float timeDelta);
	void actionAttack(float timeDelta);
	void actionRunaway(float timeDelta);
};


double GetLengthBetween(D2D1_POINT_2F p1, D2D1_POINT_2F p2);

struct D2DVECTOR
{
	float x;
	float y;
};
