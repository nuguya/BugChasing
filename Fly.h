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
	int id; //���� ������ ���� ID
	BOOL _isCatch; //������ �������� ���� Ȯ��
	int unigue; //0�̸� ������ 0�� �ƴϸ� �Ϲ� ����
	float length; //�ִϸ��̼ǿ� ���
	float anim_time = 0.0f; //�ִϸ��̼ǿ� ���
	ID2D1PathGeometry* m_FlyGeometry; //������ü�� ���� �ִ� ���ϰ��
	AnimationLinear<float> m_Animation; 
	D2D1_POINT_2F point; //������ ��ǥ
public:
	Fly(ID2D1PathGeometry* a,float duration) {
		m_FlyGeometry = a; //��� ����
		m_FlyGeometry->ComputeLength(NULL, &length); //�ִϸ��̼��� ���� ���
		m_Animation.SetStart(0);
		m_Animation.SetEnd(length);
		m_Animation.SetDuration(duration);
	}

	~Fly() {
		SAFE_RELEASE(m_FlyGeometry);
	}

};