#pragma once
#include "Animation.h"
#include "DemoApp.h"
#include <stdlib.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }

class Fly
{
public:
	int id; //각각 벌레의 고유 ID
	BOOL _isCatch; //벌레가 잡혔는지 여부 확인
	int unigue; //0이면 골드버그 0이 아니면 일반 벌레
	float length; //애니메이션에 사용
	float anim_time = 0.0f; //애니메이션에 사용
	ID2D1PathGeometry* m_FlyGeometry; //벌레객체가 갖고 있는 기하경로
	AnimationLinear<float> m_Animation; 
	D2D1_POINT_2F point; //벌레의 좌표
public:
	Fly(ID2D1PathGeometry* a,float duration) {
		m_FlyGeometry = a; //경로 결정
		m_FlyGeometry->ComputeLength(NULL, &length); //애니메이션을 위한 계산
		m_Animation.SetStart(0);
		m_Animation.SetEnd(length);
		m_Animation.SetDuration(duration);
	}

	~Fly() {
		SAFE_RELEASE(m_FlyGeometry);
	}

};