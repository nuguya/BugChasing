#define _WIN32_DCOM
#define _CRT_SECURE_NO_DEPRECATE
#include "DemoApp.h"
#include <time.h>
#include <vector>
#include <list>
#include <iostream>
#include <stdlib.h>
#include <typeinfo>
#include <math.h>
#include "Fly.h"
#include "_physics.h"
#include "FSMbug.h"
#include <windows.h>
#include "ball.h"
#define M_PI       3.14159265358979323846
#define NUM 5
#define NUM_ENEMIES		1
BOOL UNIQUE = FALSE;
float dist;
_State _state_x[NUM], _state_y[NUM] , prev_state[NUM];
XAUDIO2_VOICE_STATE state;
using namespace std;

/*---------------------------------	
ㅣ								
ㅣ	
ㅣ		전역변수
ㅣ	
ㅣ	
----------------------------------*/
float p_length = 0;
float timeDelta;
BOOL CRUSH;
BOOL SOUND = TRUE;
HANDLE hTimer = NULL;
HANDLE hTimer2 = NULL;
HANDLE hTimer3 = NULL;
HANDLE hTimer4 = NULL;
HANDLE hTimerQueue = NULL;
D2D1_POINT_2F current, start_pull[NUM], end_pull[NUM]; //현재 좌표 포인터
D2D1_POINT_2F current2;
BOOL makeBug = FALSE;
BOOL _select = FALSE; 
BOOL _catch = FALSE;
BOOL makeBug_FSM = FALSE;
BOOL _pull = FALSE; //잡아당기기
BOOL t_table[NUM] = { 0 };
BOOL T;
INT n = 0;
INT SWITCH = 0;
WCHAR szText[100]; //텍스트
list <Fly*> v;
Ball* b;
list<Character*> enemyCharacters;
Character* myCharacter = NULL;
FLOAT t[NUM], dt[NUM];
INT ti = 0;
float volume = 0.5f;
float bgm_volume = 0.2f;
int catch_num = 0; //잡은 벌레수
int current_bugNUm = 0;
int goldbug_catchNum = 0; //골드버그가 잡은 수
CWaveFile wav1;
WAVEFORMATEX* pwfx;
IXAudio2* pXAudio2 = NULL;
IXAudio2SourceVoice* pSourceVoice;

#if defined(_DEBUG)&&defined(WIN32)&&!defined(_AFX)&&!defined(_AFXDLL)
#define TRACE TRACE_WIN32
#pragma warning(disable: 4996)
void TRACE_WIN32(LPCTSTR lpszFormat, ...) {
	TCHAR lpszBuffer[0x160]; //buffer size
	va_list fmtList;
	va_start(fmtList, lpszFormat);
	_vswprintf(lpszBuffer, lpszFormat, fmtList);
	va_end(fmtList);
	OutputDebugString(lpszBuffer);
}
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif


HRESULT SoundDie(IXAudio2* pXaudio2);
HRESULT SoundWall(IXAudio2* pXaudio2);


HRESULT SoundClick(IXAudio2 * pXaudio2);

HRESULT SoundThrow(IXAudio2 * pXaudio2);

HRESULT SoundBGM(IXAudio2 * pXaudio2);

VOID CALLBACK MakeFly(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	if (current_bugNUm < 30) {
		if (rand() % 5 == 1)
			makeBug_FSM = TRUE;
		else
			makeBug = TRUE;
	}
}


VOID CALLBACK SetGoldBug(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	if (_select)
		_select = FALSE;
}

VOID CALLBACK eraseObject2(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	for (auto i = enemyCharacters.begin(); i != enemyCharacters.end(); ++i) {
		if ((*i)->id == (int)lpParam) {
			enemyCharacters.erase(i);
			break;
		}
	}
}

VOID CALLBACK eraseObject(PVOID lpParam, BOOLEAN TimerOrWaitFired)
{
	for (auto i = v.begin(); i != v.end(); ++i) {
		if ((*i)->id == (int)lpParam) {
			if ((*i)->unigue == 0)
				UNIQUE = FALSE;
			v.erase(i);
			break;
		}
	}
}


int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int /*nCmdShow*/)
{
	if (SUCCEEDED(CoInitialize(NULL)))
	{
		{
			DemoApp app;
			srand(time(NULL));
			for (int i = 0; i < NUM; ++i) {
				t[i] = 0;
				dt[i] = 0.01f;
				_state_x[i].v = 500;
				_state_y[i].v = 100;
			}
			
			if (SUCCEEDED(app.Initialize()))
			{
				app.RunMessageLoop();
			}
		}
		CoUninitialize();
	}

	return 0;
}

DemoApp::DemoApp() :
	m_hwnd(NULL), //윈도우 핸들
	m_pD2DFactory(NULL), //D2D 팩토리
	m_pRT(NULL), //렌더타겟
	m_pPathGeometry(NULL),
	m_pObjectGeometry(NULL),
	m_pPathBrush(NULL), //경로를 그릴 붓
	m_pGoldBrush(NULL),  //황금 붓
	m_Animation(), //애니메이션 객체
	m_flyBitmap(NULL), //파리비트맵
	m_FlyGeometry(), //파리 이동 경로
	m_pWICFactory(NULL),
	m_backgroundBitmap(NULL), //배경 비트맵
	m_GoldBugMask(NULL),
	m_pTextFormat(NULL),
	m_pBlackBrush(NULL),
	m_pDWriteFactory(NULL),
	m_rainbowBitmap(NULL),
	m_EffectBitmap(NULL),
	m_pEffectBrush(NULL),
	m_pRectGeo(NULL),
	pCharacter(NULL),
	pEnemy(),
    pViewRange(),
	pAttackRange()
{
}

DemoApp::~DemoApp()
{
	SAFE_RELEASE(m_pD2DFactory);
	SAFE_RELEASE(m_pRT);
	SAFE_RELEASE(m_pPathGeometry);
	SAFE_RELEASE(m_pObjectGeometry);
	SAFE_RELEASE(m_pPathBrush);
	SAFE_RELEASE(m_pGoldBrush);
	SAFE_RELEASE(m_flyBitmap);
	SAFE_RELEASE(m_pWICFactory);
	SAFE_RELEASE(m_backgroundBitmap);
	SAFE_RELEASE(m_GoldBugMask);
	SAFE_RELEASE(m_pTextFormat);
	SAFE_RELEASE(m_EffectBitmap);
	SAFE_RELEASE(m_pBlackBrush);
	SAFE_RELEASE(m_rainbowBitmap);
	SAFE_RELEASE(m_pEffectBrush);
	for (int i = 0; i<ARRAYSIZE(m_FlyGeometry); ++i)
			SAFE_RELEASE(m_FlyGeometry[i]);
	for (int i = 0; i<ARRAYSIZE(pEnemy); ++i)
		SAFE_RELEASE(pEnemy[i]);
	for (int i = 0; i<ARRAYSIZE(pViewRange); ++i)
		SAFE_RELEASE(pViewRange[i]);
	for (int i = 0; i<ARRAYSIZE(pAttackRange); ++i)
		SAFE_RELEASE(pAttackRange[i]);
}

HRESULT DemoApp::Initialize()
{
	HRESULT hr;

	//register window class
	WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = DemoApp::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = sizeof(LONG_PTR);
	wcex.hInstance = HINST_THISCOMPONENT;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = NULL;
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"D2DDemoApp";
	RegisterClassEx(&wcex);

	hr = CreateDeviceIndependentResources();
	if (SUCCEEDED(hr))
	{
		XAudio2Create(&pXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
		IXAudio2MasteringVoice* pMasterVoice = NULL;
		pXAudio2->CreateMasteringVoice(&pMasterVoice);
		SoundBGM(pXAudio2);
		b = new Ball();
		initFSM();
		++current_bugNUm;
		m_hwnd = CreateWindow(
			L"D2DDemoApp", L"벌레잡기", WS_OVERLAPPEDWINDOW, 0,0,
			840, 680, NULL, NULL, HINST_THISCOMPONENT, this);
		hr = m_hwnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{
			if (SUCCEEDED(hr))
			{
				ZeroMemory(&m_DwmTimingInfo, sizeof(m_DwmTimingInfo));
				m_DwmTimingInfo.cbSize = sizeof(m_DwmTimingInfo);
				hr = DwmGetCompositionTimingInfo(NULL, &m_DwmTimingInfo);

				if (FAILED(hr)) {
					HDC hdc = GetDC(m_hwnd);
					m_DwmTimingInfo.rateCompose.uiDenominator = 1;
					m_DwmTimingInfo.rateCompose.uiNumerator = GetDeviceCaps(hdc, VREFRESH);
					ReleaseDC(m_hwnd, hdc);
				}

				ShowWindow(m_hwnd, SW_SHOWNORMAL);
				UpdateWindow(m_hwnd);
			}
		}
	}

	//QueryPerformanceFrequency(&m_nFrequency);
	//QueryPerformanceCounter(&m_nPrevTime);

	return hr;
}

HRESULT DemoApp::CreateDeviceIndependentResources()
{
	HRESULT hr;
	ID2D1GeometrySink* pSink = NULL;

	// D2D 팩토리를 생성함.
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);

	// WIC 팩토리를 생성함.
	if (SUCCEEDED(hr))
	{
		hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pWICFactory));
	}
	if (SUCCEEDED(hr))
	{
		/*Text를 위한 자원생성*/
		DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(m_pDWriteFactory),
			reinterpret_cast<IUnknown **>(&m_pDWriteFactory));
			m_pDWriteFactory->CreateTextFormat(L"Verdana", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 20, L"", &m_pTextFormat);
			m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
			m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	}
	if (SUCCEEDED(hr))
	{
		hr = m_pD2DFactory->CreateRectangleGeometry(D2D1::RectF(0, 0, 70, 70), &m_pRectGeo);
	}
	
	if (SUCCEEDED(hr))
	{
		m_pD2DFactory->CreatePathGeometry(&pCharacter);
		hr = pCharacter->Open(&pSink);
	}
	if (SUCCEEDED(hr))
	{
		D2D1_POINT_2F StartPos = D2D1::Point2F(-10, 0);
		pSink->BeginFigure(StartPos, D2D1_FIGURE_BEGIN_FILLED);

		pSink->AddArc(D2D1::ArcSegment(D2D1::Point2F(10, 0), D2D1::SizeF(10, 10), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
		pSink->AddArc(D2D1::ArcSegment(D2D1::Point2F(-10, 0), D2D1::SizeF(10, 10), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));

		pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

		hr = pSink->Close();
	}
	SAFE_RELEASE(pSink);

	for (int i = 0; i < 5; i++)
	{
		hr = m_pD2DFactory->CreatePathGeometry(&pEnemy[i]);
		hr = m_pD2DFactory->CreatePathGeometry(&pViewRange[i]);
		hr = m_pD2DFactory->CreatePathGeometry(&pAttackRange[i]);
		//Enemy
		if (SUCCEEDED(hr))
		{
			hr = pEnemy[i]->Open(&pSink);
		}
		if (SUCCEEDED(hr))
		{
			D2D1_POINT_2F StartPos = D2D1::Point2F(-10, 0);
			pSink->BeginFigure(StartPos, D2D1_FIGURE_BEGIN_FILLED);

			pSink->AddArc(D2D1::ArcSegment(D2D1::Point2F(10, 0), D2D1::SizeF(10, 10), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
			pSink->AddArc(D2D1::ArcSegment(D2D1::Point2F(-10, 0), D2D1::SizeF(10, 10), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));

			pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

			hr = pSink->Close();
		}
		SAFE_RELEASE(pSink);
		//ViewRange
		if (SUCCEEDED(hr))
		{
			hr = pViewRange[i]->Open(&pSink);
		}
		if (SUCCEEDED(hr))
		{
			D2D1_POINT_2F StartPos = D2D1::Point2F(-90, 0);
			pSink->BeginFigure(StartPos, D2D1_FIGURE_BEGIN_FILLED);

			pSink->AddArc(D2D1::ArcSegment(D2D1::Point2F(90, 0), D2D1::SizeF(90, 90), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
			pSink->AddArc(D2D1::ArcSegment(D2D1::Point2F(-90, 0), D2D1::SizeF(90, 90), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));

			pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

			hr = pSink->Close();
		}
		SAFE_RELEASE(pSink);
		//AttackRange
		if (SUCCEEDED(hr))
		{
			hr = pAttackRange[i]->Open(&pSink);
		}
		if (SUCCEEDED(hr))
		{
			D2D1_POINT_2F StartPos = D2D1::Point2F(-30, 0);
			pSink->BeginFigure(StartPos, D2D1_FIGURE_BEGIN_FILLED);

			pSink->AddArc(D2D1::ArcSegment(D2D1::Point2F(30, 0), D2D1::SizeF(30, 30), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
			pSink->AddArc(D2D1::ArcSegment(D2D1::Point2F(-30, 0), D2D1::SizeF(30, 30), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));

			pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

			hr = pSink->Close();
		}
		SAFE_RELEASE(pSink);
	}


	//날파리가 날아다니는 경로기하 1번 (임의 설정한 직선 경로)
	if (SUCCEEDED(hr))
	{
		hr = m_pD2DFactory->CreatePathGeometry(&m_FlyGeometry[0]); 
		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink* pSink = NULL;
			hr = m_FlyGeometry[0]->Open(&pSink);
			if (SUCCEEDED(hr))
			{
				pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

				pSink->BeginFigure(D2D1::Point2F(346, 255), D2D1_FIGURE_BEGIN_FILLED);
				D2D1_POINT_2F points[10] = {
					D2D1::Point2F(267, 177),
					D2D1::Point2F(236, 192),
					D2D1::Point2F(212, 760),
					D2D1::Point2F(156, 255),
					D2D1::Point2F(346, 255),
					D2D1::Point2F(546, 135),
					D2D1::Point2F(26, 555),
					D2D1::Point2F(321, 55),
					D2D1::Point2F(746, 655),
					D2D1::Point2F(100, 455),
				};
				pSink->AddLines(points, ARRAYSIZE(points));
				pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

			}
			hr = pSink->Close();

			SAFE_RELEASE(pSink);
		}
	}
	//날파리가 날아다니는 경로기하 2번 (지그재그 경로)
	if (SUCCEEDED(hr))
	{
		hr = m_pD2DFactory->CreatePathGeometry(&m_FlyGeometry[1]); 
		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink* pSink = NULL;

			hr = m_FlyGeometry[1]->Open(&pSink);
			if (SUCCEEDED(hr))
			{
				pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

				pSink->BeginFigure(D2D1::Point2F(0, 0), D2D1_FIGURE_BEGIN_FILLED);
				D2D1_POINT_2F points[] = {
					D2D1::Point2F(0,0),
					D2D1::Point2F(740, 0),
					D2D1::Point2F(0, 80),
					D2D1::Point2F(740, 80),
					D2D1::Point2F(0, 160),
					D2D1::Point2F(740, 160),
					D2D1::Point2F(0, 240),
					D2D1::Point2F(740, 240),
					D2D1::Point2F(0, 320),
					D2D1::Point2F(740, 320),
					D2D1::Point2F(0, 400),
					D2D1::Point2F(740, 400),
					D2D1::Point2F(0, 480),
					D2D1::Point2F(740, 480),
					D2D1::Point2F(0, 560),
					D2D1::Point2F(740, 560),
					D2D1::Point2F(0, 640),
					D2D1::Point2F(740, 640),
					D2D1::Point2F(0,640),
				};
				pSink->AddLines(points, ARRAYSIZE(points));
				pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
			}
			hr = pSink->Close();

			SAFE_RELEASE(pSink);
		}
	}
	//날파리가 날아다니는 경로기하 3번 (나선형 모양의 경로)
	if (SUCCEEDED(hr))
	{
		hr = m_pD2DFactory->CreatePathGeometry(&m_FlyGeometry[2]);
		if (SUCCEEDED(hr))
		{
			hr = m_FlyGeometry[2]->Open(&pSink);
		}
		if (SUCCEEDED(hr))
		{
			D2D1_POINT_2F currentLocation = { 350, 300 };

			pSink->BeginFigure(currentLocation, D2D1_FIGURE_BEGIN_FILLED);

			D2D1_POINT_2F locDelta = { 2, 2 };
			float radius = 3;

			for (UINT i = 0; i < 30; ++i)
			{
				currentLocation.x += radius * locDelta.x;
				currentLocation.y += radius * locDelta.y;

				pSink->AddArc(
					D2D1::ArcSegment(
						currentLocation,
						D2D1::SizeF(2 * radius, 2 * radius), // radiusx/y
						0.0f, // rotation angle
						D2D1_SWEEP_DIRECTION_CLOCKWISE,
						D2D1_ARC_SIZE_SMALL
					)
				);

				locDelta = D2D1::Point2F(-locDelta.y, locDelta.x);

				radius += 5;
			}
			D2D1_POINT_2F points[] = {
				D2D1::Point2F(350,300) };
			pSink->AddLines(points, ARRAYSIZE(points));
			pSink->EndFigure(D2D1_FIGURE_END_OPEN);

			hr = pSink->Close();
		}

		SAFE_RELEASE(pSink);
	}
	//날파리가 날아다니는 경로기하 4번 (베지어 곡선을 이용한 경로)
	if (SUCCEEDED(hr))
	{
		hr = m_pD2DFactory->CreatePathGeometry(&m_FlyGeometry[3]); //날파리가 날아다니는 경로기하 1번 (임의 설정한 직선 경로)
		if (SUCCEEDED(hr))
		{
			ID2D1GeometrySink* pSink = NULL;
			hr = m_FlyGeometry[3]->Open(&pSink);
			if (SUCCEEDED(hr))
			{
				pSink->SetFillMode(D2D1_FILL_MODE_WINDING);

				pSink->BeginFigure(D2D1::Point2F(5, 5), D2D1_FIGURE_BEGIN_FILLED);
				pSink->AddBezier(D2D1::BezierSegment(D2D1::Point2F(5, 5), D2D1::Point2F(12, 59), D2D1::Point2F(64, 57)));
				pSink->AddBezier(D2D1::BezierSegment(D2D1::Point2F(64, 57), D2D1::Point2F(134, 57), D2D1::Point2F(64, 107)));
				pSink->AddBezier(D2D1::BezierSegment(D2D1::Point2F(64, 107), D2D1::Point2F(34, 127), D2D1::Point2F(123, 157)));
				pSink->AddBezier(D2D1::BezierSegment(D2D1::Point2F(123, 157), D2D1::Point2F(303, 232), D2D1::Point2F(143, 307)));
				pSink->AddBezier(D2D1::BezierSegment(D2D1::Point2F(143, 307), D2D1::Point2F(25,359), D2D1::Point2F(183, 407)));
				pSink->AddBezier(D2D1::BezierSegment(D2D1::Point2F(183, 407), D2D1::Point2F(503, 507), D2D1::Point2F(503, 117)));
				pSink->AddBezier(D2D1::BezierSegment(D2D1::Point2F(503, 117), D2D1::Point2F(513, 0), D2D1::Point2F(353, 57)));
				pSink->AddBezier(D2D1::BezierSegment(D2D1::Point2F(353, 57), D2D1::Point2F(303, 75), D2D1::Point2F(453, 157)));
				pSink->AddBezier(D2D1::BezierSegment(D2D1::Point2F(453, 157), D2D1::Point2F(613, 232), D2D1::Point2F(503, 357)));
				pSink->AddBezier(D2D1::BezierSegment(D2D1::Point2F(503, 357), D2D1::Point2F(413, 470), D2D1::Point2F(353, 257)));
				pSink->AddBezier(D2D1::BezierSegment(D2D1::Point2F(353, 257), D2D1::Point2F(253, 32), D2D1::Point2F(653, 157)));
				pSink->AddBezier(D2D1::BezierSegment(D2D1::Point2F(653, 157), D2D1::Point2F(823, 213), D2D1::Point2F(453, 507)));
				pSink->AddBezier(D2D1::BezierSegment(D2D1::Point2F(453, 507), D2D1::Point2F(75, 770), D2D1::Point2F(253, 407)));
				pSink->AddBezier(D2D1::BezierSegment(D2D1::Point2F(253, 407), D2D1::Point2F(403, 53), D2D1::Point2F(703, 357)));
				pSink->AddBezier(D2D1::BezierSegment(D2D1::Point2F(703, 357), D2D1::Point2F(880, 537), D2D1::Point2F(803, 327)));
				pSink->AddBezier(D2D1::BezierSegment(D2D1::Point2F(803, 327), D2D1::Point2F(643, -10), D2D1::Point2F(5, 5)));
				pSink->EndFigure(D2D1_FIGURE_END_OPEN);

			}
			hr = pSink->Close();

			SAFE_RELEASE(pSink);
		}
	}
	//날파리가 날아다니는 경로기하 5번 (

	return hr;
}

HRESULT DemoApp::CreateDeviceResources()
{
	HRESULT hr = S_OK;

	if (!m_pRT)
	{
		RECT rc;
		GetClientRect(m_hwnd, &rc);

		D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

		// 사용될 비트맵들을 로드함.
		hr = m_pD2DFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(m_hwnd, size), &m_pRT);

		hr = LoadBitmapFromResource(m_pRT, m_pWICFactory, L"FlyImage", L"Image", 100, 0, &m_flyBitmap); //파리이미지

		hr = LoadBitmapFromResource(m_pRT, m_pWICFactory, L"BackgroundImage1", L"Image", 0, 0, &m_backgroundBitmap); //배경이미지

		hr = LoadBitmapFromResource(m_pRT, m_pWICFactory, L"BugMask", L"Image", 0, 0, &m_GoldBugMask); //황금벌레 이미지 마스크

		hr = LoadBitmapFromResource(m_pRT, m_pWICFactory, L"RainbowImage", L"Image", 150, 150, &m_rainbowBitmap); //무지개이미지

		hr = LoadBitmapFromResource(m_pRT, m_pWICFactory, L"EffectImage", L"Image", 200, 200, &m_EffectBitmap); //효과이미지

		D2D1_BITMAP_BRUSH_PROPERTIES propertiesXClampYClamp = D2D1::BitmapBrushProperties(D2D1_EXTEND_MODE_CLAMP, D2D1_EXTEND_MODE_CLAMP, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
		// 사용된 붓들을 생성함.
		if (SUCCEEDED(hr))
		{
			hr = m_pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_pBlackBrush);
		}
		if (SUCCEEDED(hr))
		{
			hr = m_pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White,0.0f), &m_pPathBrush);
		}
		if (SUCCEEDED(hr))
		{
			hr = m_pRT->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gold), &m_pGoldBrush);
		}
		if (SUCCEEDED(hr))
		{
			hr = m_pRT->CreateBitmapBrush(m_rainbowBitmap, propertiesXClampYClamp, &m_pBitmapBrush);
		}
		if(SUCCEEDED(hr))
		{
			hr = m_pRT->CreateBitmapBrush(m_EffectBitmap, propertiesXClampYClamp, &m_pEffectBrush);
		}
		if (SUCCEEDED(hr))
		{
			ID2D1GradientStopCollection* pGradientStops = NULL;

			static const D2D1_GRADIENT_STOP gradientStops[] =
			{
				{ 0.f,  D2D1::ColorF(D2D1::ColorF::Black, 1.0f) },
			{ 1.f,  D2D1::ColorF(D2D1::ColorF::White, 0.0f) },
			};

			hr = m_pRT->CreateGradientStopCollection(gradientStops, 2, &pGradientStops);

			if (SUCCEEDED(hr))
			{
				hr = m_pRT->CreateLinearGradientBrush(
					D2D1::LinearGradientBrushProperties(D2D1::Point2F(0, 0), D2D1::Point2F(150, 150)),
					pGradientStops, &m_pLinearGradientBrush);
			}

			if (SUCCEEDED(hr))
			{
				hr = m_pRT->CreateRadialGradientBrush(
					D2D1::RadialGradientBrushProperties(D2D1::Point2F(75, 75), D2D1::Point2F(0, 0), 75, 75),
					pGradientStops, &m_pRadialGradientBrush);
			}
			pGradientStops->Release();
		}
	}

	return hr;
}

void DemoApp::DiscardDeviceResources()
{
	SAFE_RELEASE(m_pRT);
	SAFE_RELEASE(m_pPathBrush);
	SAFE_RELEASE(m_pGoldBrush);
	SAFE_RELEASE(m_flyBitmap);
	SAFE_RELEASE(m_backgroundBitmap);
	SAFE_RELEASE(m_GoldBugMask);
	SAFE_RELEASE(m_pBitmapBrush);
	SAFE_RELEASE(m_rainbowBitmap);
	SAFE_RELEASE(m_EffectBitmap);
	SAFE_RELEASE(m_pEffectBrush);
}

void DemoApp::RunMessageLoop()
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

bool DemoApp::updateFSM(float timeDelta)
{
	for (std::list<Character*>::iterator iter = enemyCharacters.begin(); iter != enemyCharacters.end(); iter++)
	{
		Character* ch = *iter;
		ch->update(b, timeDelta);
	}
	// 내 위치를 갱신함.
	return true;
}

void DemoApp::initFSM()
{
	// 초기 위치들을 중복되지 않게 정한다.
	int posx[NUM_ENEMIES], posy[NUM_ENEMIES], cx, cy;
	int k = 0;
	bool duplicated;
	for (int i = 0; i<NUM_ENEMIES; i++)
	{
		do {
			cx = rand() % (2 * PLANE_SIZE_X) - PLANE_SIZE_X;
			cy = rand() % (2 * PLANE_SIZE_Y) - PLANE_SIZE_Y;
			duplicated = false;
			if (cx <0 || cy <0 || cx >640 || cy>480)
				duplicated = true;
			if (i != 0)
			{
				for (int j = 0; j<i; j++)
				{
					if (sqrt((float)(cx - posx[i])*(cx - posx[i]) + (float)(cy - posy[i])*(cy - posy[i])) < 10)
					{
						duplicated = true;
						break;
					}
				}
			}
		} while (duplicated);
		posx[k] = cx; posy[k] = cy; k++;
	}

	// 적군 캐릭터들을 생성함.
	for (int i = 0; i<NUM_ENEMIES; i++)
	{
		DWORD enemyStateTransitions[][3] = {
		{ Character::STATE_STAND, Character::EVENT_FINDTARGET, Character::STATE_RUNAWAY },
		{ Character::STATE_STAND, Character::EVENT_DUBIOUS, Character::STATE_MOVE },
		{ Character::STATE_MOVE, Character::EVENT_FINDTARGET, Character::STATE_RUNAWAY },
		{ Character::STATE_MOVE, Character::EVENT_STOPWALK, Character::STATE_STAND },
		{ Character::STATE_FOLLOW, Character::EVENT_LOSTTARGET, Character::STATE_STAND },
		{ Character::STATE_RUNAWAY, Character::EVENT_LOSTTARGET, Character::STATE_IDLE },
		{ Character::STATE_IDLE, Character::EVENT_FINDTARGET, Character::STATE_RUNAWAY },
		{ Character::STATE_IDLE, Character::EVENT_DUBIOUS, Character::STATE_MOVE },
		};

		Character* enemy = new Character(Character::TYPE_AI, enemyStateTransitions, 8);
		enemy->id = n++;
		enemy->setPosition(D2D1::Point2F((float)posx[i], (float)posy[i]));
		enemyCharacters.push_back(enemy);

		{
			DWORD myStateTransitions[][3] =
			{
				{ Character::STATE_STAND, Character::EVENT_FINDTARGET, Character::STATE_MOVE },
			{ Character::STATE_MOVE, Character::EVENT_STOPWALK, Character::STATE_STAND },
			};
		}
	}


}

bool intersectPlane(int sx, int sy)
{
	D2D1_POINT_2F picked = D2D1::Point2F((float)sx, (float)sy);
	myCharacter->setDestPosition(picked);
	return true;
}

HRESULT DemoApp::OnRender()
{
	HRESULT hr;
	static int i=0;

	hr = CreateDeviceResources();
	if (SUCCEEDED(hr))
	{

		// 그리기를 준비함.
		m_pRT->BeginDraw();
		
		// 변환을 항등행렬로 리셋함.
		m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());

		m_pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));
		m_pRT->DrawBitmap(m_backgroundBitmap, D2D1::RectF(840.0f, 840.0f));
		
		/*유한상태머신 생성*/
		for (list <Character*>::iterator b_iter = enemyCharacters.begin(); b_iter != enemyCharacters.end(); ++b_iter,++i)
		{
			FSM(b_iter,i=i%5);
		}
		updateFSM(static_cast<float>(m_DwmTimingInfo.rateCompose.uiDenominator) /
			static_cast<float>(m_DwmTimingInfo.rateCompose.uiNumerator));
		DrawBall(b);
		if (_pull) { //공을 당길 때 라인을 그림.
			m_pRT->DrawLine(start_pull[0], current2, m_pBlackBrush);
			dist = sqrt(pow((end_pull[0].x - start_pull[0].x), 2) + pow((end_pull[0].y - start_pull[0].y), 2));
		}
		Throw(0);
		
		/*텍스트 출력*/
		_swprintf(szText, L"현재 벌레의 수 : %d\n잡은 벌레의 수 : %d", current_bugNUm,catch_num);
		m_pRT->DrawTextW(szText, wcslen(szText), m_pTextFormat, D2D1::RectF(0, 0, 200, 100),m_pBitmapBrush);

		if (_select)
			GoldBug();
		/*생성된 벌레 움직임*/
		for (auto b_iter=v.begin(); b_iter!=v.end(); ++b_iter) {
			m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());
			FlyMove(b_iter);
		}
		hr = m_pRT->EndDraw();
	}

	return hr;
}

bool intersectPlane(int sx, int sy);

float DemoApp::_line(D2D1_POINT_2F p1, D2D1_POINT_2F p2) //직선의 기울기를 구하기 위한 함수.
{
	float temp;
	if (p2.x == p1.x)
		p2.x += 0.01; //에러방지
	float m = (p2.y - p1.y) / (p2.x - p1.x);

	return m;
}

void DemoApp::FSM(list <Character*>::iterator iter,int i)
{
	D2D1_MATRIX_3X2_F triangleMatrix;
	float minWidthHeightScale = 1;
	D2D1::Matrix3x2F Trans;
	D2D1::Matrix3x2F scale = D2D1::Matrix3x2F::Scale(minWidthHeightScale, minWidthHeightScale);
	D2D1::Matrix3x2F translation = D2D1::Matrix3x2F::Translation(0, 0);
	if ((*iter)->_isCatch == TRUE) {
		m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());
		m_pRT->DrawBitmap(m_EffectBitmap, D2D1::RectF((*iter)->deadpoint.x - 15, (*iter)->deadpoint.y - 15, (*iter)->deadpoint.x + 15, (*iter)->deadpoint.y + 15));
	}
	else {
		if (((*iter)->getPosition().x > (*iter)->getDestPositon().x))
			(*iter)->refangle = 180;
		else
			(*iter)->refangle = 0;
		float angle = atan(((*iter)->getDestPositon().y - (*iter)->getPosition().y) / ((*iter)->getDestPositon().x - (*iter)->getPosition().x))*(180 / M_PI) + (*iter)->refangle;
		triangleMatrix = D2D1::Matrix3x2F::Rotation(angle, D2D1::Point2F(10.0f, 10.0f));
		Trans = D2D1::Matrix3x2F::Translation((*iter)->getPosition().x, (*iter)->getPosition().y);
		m_pRT->SetTransform(triangleMatrix*scale*Trans);
		m_pRT->DrawBitmap(m_flyBitmap, D2D1::RectF(20.0f, 20.0f));
		if(T)
			m_pRT->DrawGeometry(pViewRange[i], m_pBitmapBrush, 0.3f);
		m_pRT->SetTransform(D2D1::Matrix3x2F::Identity());
	}
}

void DemoApp::DrawBall(Ball *ball)
{
	D2D1_POINT_2F ep = {ball->point.x,ball->point.y};
	m_pRT->FillEllipse(D2D1::Ellipse(ep, 5, 5), m_pBlackBrush);
}

void DemoApp::Throw(int idx)
{
	D2D1_POINT_2F r;
	float t1, t2;
	float m = _line(start_pull[idx], end_pull[idx]);

	if (SWITCH > 0) {
		if (start_pull[idx].x<end_pull[idx].x&&start_pull[idx].y<end_pull[idx].y) {
			integrate(_state_x[idx], t[idx], dt[idx], 0);
			integrate(_state_y[idx], t[idx], dt[idx], 0);
		}
		else if (start_pull[idx].x<end_pull[idx].x&&start_pull[idx].y>end_pull[idx].y){
			integrate(_state_x[idx], t[idx], dt[idx], 0);
			integrate(_state_y[idx], t[idx], dt[idx], 1);
		}
		else if (start_pull[idx].x>end_pull[idx].x&&start_pull[idx].y>end_pull[idx].y) {
			integrate(_state_x[idx], t[idx], dt[idx], 1);
			integrate(_state_y[idx], t[idx], dt[idx], 1);
		}
		else if (start_pull[idx].x>end_pull[idx].x&&start_pull[idx].y<end_pull[idx].y) {
			integrate(_state_x[idx], t[idx], dt[idx], 1);
			integrate(_state_y[idx], t[idx], dt[idx], 0);
		}
		else {
			b->state = FALSE;
		}
		if (abs(m) > 1) { //m은 기울기,start_pull[idx].x는 시작좌표
			r.y = _state_y[idx].x;
			r.x = (r.y - start_pull[idx].y) / m + start_pull[idx].x; //직선의 방정식
		}
		else {
			r.x = _state_x[idx].x; 
			r.y = m * (r.x - start_pull[idx].x) + start_pull[idx].y;
		}
		//_state_y[idx].x = r.y;
		if (r.x < 0) {
			r.x *= -1;
			//_state_x[idx].v *= -1;

		}
		if (r.y < 0) {
			r.y = (-1 * r.y);
			//_state_y[idx].v *= -1;
		}
		if (r.x > 840) {
			r.x = 840 - (int)r.x % 840;
			//_state_x[idx].v *= -1;
		}
		if (r.y > 640) {
			r.y = 640 - (int)r.y % 640;
			//_state_y[idx].v *= -1;
		}
		
		if (abs(_state_x[idx].v) < 10||abs(_state_y[idx].v)<10) {
			b->state = FALSE;
			SWITCH=0;
			dt[0] = 0.0f;
		}
		b->point.x = r.x; b->point.y = r.y;
		for (auto i = v.begin(); i != v.end(); ++i) {
			if (((*i)->point.x + 10 >= b->point.x && (*i)->point.x - 10 <= b->point.x) && ((*i)->point.y + 10 >= b->point.y && (*i)->point.y - 10 <= b->point.y) && (*i)->_isCatch != TRUE) {
				if (b->state) {
					SoundDie(pXAudio2);
					(*i)->_isCatch = TRUE;
					CreateTimerQueueTimer(&hTimer3, hTimerQueue, eraseObject, (PVOID)((*i)->id), 5000, 0, WT_EXECUTEONLYONCE);
					++catch_num;
					--current_bugNUm;
				}
			}
		}
		t[idx] += dt[idx];
	}
		
		

}

void DemoApp::GoldBug()
{
	D2D1_RECT_F rcBrushRect = D2D1::RectF(current.x - 25, current.y - 25, current.x + 25, current.y + 25);

	m_pRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
	m_pRT->FillOpacityMask(
		m_GoldBugMask,
		m_pBitmapBrush,
		D2D1_OPACITY_MASK_CONTENT_GRAPHICS,
		&rcBrushRect);
	m_pRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
}

void DemoApp::FlyMove(list <Fly*>::iterator iter)
{
	HRESULT hr;
	D2D1_POINT_2F point;
	D2D1_POINT_2F tangent;
	D2D1_MATRIX_3X2_F triangleMatrix;
	D2D1_RECT_F rcBrushRect = D2D1::RectF(0,0,50,50);
	D2D1::Matrix3x2F translation = D2D1::Matrix3x2F::Translation(0, 0);
	D2D1_SIZE_F rtSize = DemoApp::m_pRT->GetSize();
	float minWidthHeightScale = 1;

	D2D1::Matrix3x2F scale = D2D1::Matrix3x2F::Scale(minWidthHeightScale, minWidthHeightScale);
	if ((*iter)->_isCatch == TRUE) {
		m_pRT->DrawBitmap(m_EffectBitmap,D2D1::RectF((*iter)->point.x-15, (*iter)->point.y-15, (*iter)->point.x + 15, (*iter)->point.y + 15));
	}
	else{
		float length = (*iter)->m_Animation.GetValue((*iter)->anim_time);
		// 현재 시간에 해당하는 기하 길이에 일치하는 이동 동선 상의 지점을 얻음.
		(*iter)->m_FlyGeometry->ComputePointAtLength(length, NULL, &point, &tangent);
		// 경로에 따른 파리의 방향결정.
		triangleMatrix = D2D1::Matrix3x2F(
			tangent.x, tangent.y,
			-tangent.y, tangent.x,
			point.x, point.y);
		
		m_pRT->SetTransform(triangleMatrix * scale * translation);
		(*iter)->point.x = point.x; (*iter)->point.y = point.y;

		if ((*iter)->unigue) //벌레가 일반 벌레일 때
			m_pRT->DrawBitmap(m_flyBitmap, D2D1::RectF(20.0f, 20.0f));
		else { //벌레가 골드버그일 때 불투명 마스크를 이용하여 사슴벌레 모양을 골드색으로 칠한다.
			m_pRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
			m_pRT->FillOpacityMask(
				m_GoldBugMask,
				m_pGoldBrush,
				D2D1_OPACITY_MASK_CONTENT_GRAPHICS,
				&rcBrushRect);
			m_pRT->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		}

		if (hr == D2DERR_RECREATE_TARGET)
		{
			hr = S_OK;
			DiscardDeviceResources();
		}

		//애니메이션의 끝에 도달하면 다시 처음으로 되돌려서 반복되도록 함.
		if ((*iter)->anim_time >= (*iter)->m_Animation.GetDuration())
		{
			(*iter)->anim_time = 0.0f;
		}
		else
		{
			float elapsedTime = 0.0; //DWM 사용하여 경과시간을 구함.
			elapsedTime += static_cast<float>(m_DwmTimingInfo.rateCompose.uiDenominator) /
				static_cast<float>(m_DwmTimingInfo.rateCompose.uiNumerator);
			(*iter)->anim_time += elapsedTime;
		}
	}
}

void DemoApp::OnResize(UINT width, UINT height)
{
	if (m_pRT)
	{
		D2D1_SIZE_U size;
		size.width = width;
		size.height = height;

		m_pRT->Resize(size);
	}
}



LRESULT CALLBACK DemoApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;
	LPMINMAXINFO mmi;

	/*오디오 설정*/


	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		DemoApp* pDemoApp = (DemoApp*)pcs->lpCreateParams;
		/*타이머 큐를 생성 5초마다 벌레 생성.*/
		hTimerQueue = CreateTimerQueue();
		CreateTimerQueueTimer(&hTimer, hTimerQueue,MakeFly, NULL, 5000, 5000, WT_EXECUTEDEFAULT);

		/*첫 시작시 벌레 1마리 생성*/
		Fly *fly = new Fly(pDemoApp->m_FlyGeometry[0],rand()%10+4);
		fly->unigue = rand() % 20;
		fly->id = n++;
		++current_bugNUm;
		v.push_back(fly);

		SetWindowLongPtrW(hwnd, GWLP_USERDATA, PtrToUlong(pDemoApp));

		result = 1;
	}
	else
	{
		DemoApp* pDemoApp = reinterpret_cast<DemoApp*>(static_cast<LONG_PTR>(GetWindowLongPtrW(hwnd, GWLP_USERDATA)));

		bool wasHandled = false;

		if (pDemoApp)
		{
			switch(message)
			{
			case WM_SIZE:
				{
					UINT width = LOWORD(lParam);
					UINT height = HIWORD(lParam);
					pDemoApp->OnResize(width, height);
				}
				result = 0;
				wasHandled = true;
				break;

			case WM_DISPLAYCHANGE:
				{
					InvalidateRect(hwnd, NULL, FALSE);
				}
				result = 0;
				wasHandled = true;
				break;
		
			case WM_KEYDOWN:
			{
				if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
					if (SOUND)
						pSourceVoice->Stop();
					else
						pSourceVoice->Start(0);
					SOUND = !SOUND;
				}
				if (GetAsyncKeyState(VK_F2) & 0x8000) {
					if(volume<2)
						volume += 0.1f;
					if (bgm_volume < 1.5) {
						bgm_volume += 0.1f;
						pSourceVoice->SetVolume(bgm_volume);
					}
				}
				if (GetAsyncKeyState(VK_F1) & 0x8000) {
					if(volume>0)
						volume -= 0.1f; {
						if (bgm_volume > 0)
							bgm_volume -= 0.1f;
						pSourceVoice->SetVolume(bgm_volume);
					}
				}
				if (GetAsyncKeyState(VK_F3) & 0x8000) {
					T = !T;
				}
			}
			result = 0;
			wasHandled = true;
			break;

			case WM_PAINT:
				{
					if (makeBug) { //타이머가 함수가 설정한 makeBug 플래그가 on 일때만 벌레 생성.
						Fly *fly = new Fly(pDemoApp->m_FlyGeometry[rand()%4], rand() % 10 + 30); //벌레 생성시 duration을 랜덤 값으로 줘서 각 벌레마다 속도가 다르게 생성
						fly->id = n++;
						if (UNIQUE = FALSE) {
							fly->unigue % 20;
							UNIQUE = TRUE;
						}
						v.push_back(fly);
						++current_bugNUm;
						makeBug = false;
					}
					if (makeBug_FSM) {
						initFSM();
						++current_bugNUm;
						makeBug_FSM = false;
					}

					if ((b->point.x > 837 || b->point.x < 3) || (b->point.y > 637 || b->point.y < 3)) {
						if (!CRUSH)
							if(SOUND)
								SoundWall(pXAudio2);
					}
					else
						CRUSH = FALSE;

					
					pSourceVoice->GetState(&state);
					if (state.BuffersQueued == 0)
						SoundBGM(pXAudio2);
					pDemoApp->OnRender();
					InvalidateRect(hwnd,NULL,FALSE);
					
				}
				result = 0;
				wasHandled = true;
				break;
			case WM_LBUTTONDOWN:
				{
					current.x = LOWORD(lParam); //현재 마우스 클릭시 좌표를 얻어옴
					current.y = HIWORD(lParam);
					/*공을 선택*/
					if ((current.x <= b->point.x + 10 && current.x >= b->point.x - 10) && (current.y <= b->point.y + 10 && current.y >= b->point.y - 10)&&!b->state) {
						if(SOUND)
							SoundClick(pXAudio2);
						b->isselect = TRUE;
						b->state = TRUE;
						_pull = TRUE;
						_state_x[0].v = 500;
						_state_y[0].v = 500;
						start_pull[0] = current;
						t[0] = 0;
						dt[0] = 0.01f;
					}
					/*벌레제거 부분*/
					for (list<Fly*>::iterator i = v.begin(); i!=v.end();++i) { //모든 객체를 순회하면서 현재좌표값과 일치하는 좌표가 있으면 객체제거.
						if ((*i)->unigue == 0) { //골드버그를 드래그하여 벌레제거
							if (((*i)->point.x + 50 >= current.x&&(*i)->point.x - 50 <= current.x) && ((*i)->point.y + 50 >= current.y&&(*i)->point.y - 50 <= current.y)) {
								v.erase(i);
								--current_bugNUm;
								_select = TRUE;
								CreateTimerQueueTimer(&hTimer2, hTimerQueue, SetGoldBug, NULL, 2500, 0, WT_EXECUTEDEFAULT);
								break;
							}
						} //마우스 클릭으로 벌레제거 
						else if (((*i)->point.x + 10 >= current.x&&(*i)->point.x - 10 <= current.x) && ((*i)->point.y + 10 >= current.y&&(*i)->point.y - 10 <= current.y)&&(*i)->_isCatch!=TRUE) {
							if(SOUND)
								SoundDie(pXAudio2);
							(*i)->_isCatch = TRUE;
							CreateTimerQueueTimer(&hTimer3, hTimerQueue, eraseObject, (PVOID)((*i)->id), 5000, 0, WT_EXECUTEONLYONCE);
							++catch_num;
							--current_bugNUm;
						}
					}//유한상태 벌레 제거
					for (list<Character*>::iterator i = enemyCharacters.begin(); i != enemyCharacters.end(); i++) {
						if(((*i)->position.x + 10 >= current.x && (*i)->position.x - 10 <= current.x) && ((*i)->position.y + 10 >= current.y && (*i)->position.y - 10 <= current.y) && (*i)->_isCatch != TRUE) {
							if(SOUND)
								SoundDie(pXAudio2);
							(*i)->_isCatch = TRUE;
							(*i)->deadpoint.x = current.x; (*i)->deadpoint.y = current.y;
							CreateTimerQueueTimer(&hTimer4, hTimerQueue, eraseObject2, (PVOID)((*i)->id), 5000, 0, WT_EXECUTEONLYONCE);
							++catch_num;
							--current_bugNUm;
						}
					}
				}
				result = 0;
				wasHandled = true;
				break;
			case WM_LBUTTONUP:
				{
				if (_pull) {
					t_table[ti] = TRUE;
					SWITCH += 1;
					_state_x[0].x = start_pull[0].x;
					_state_y[0].x = start_pull[0].y;
					end_pull[0].x = current2.x;
					end_pull[0].y = current2.y;
					if(SOUND)
						SoundThrow(pXAudio2);
					_pull = FALSE;
					b->isselect = FALSE;
				}
				}
				result = 0;
				wasHandled = true;
				break;
			case WM_MOUSEMOVE:
				{
					current2.x = LOWORD(lParam);
					current2.y = HIWORD(lParam);
					current.x = LOWORD(lParam); //현재 마우스 이동 시 좌표를 얻어옴
					current.y = HIWORD(lParam);
					if (_select) {
						for (auto i = v.begin(); i !=v.end();++i) {
							if (((*i)->point.x + 25 >= current.x&&(*i)->point.x - 25 <= current.x) && ((*i)->point.y + 25 >= current.y&&(*i)->point.y - 25 <= current.y)&& (*i)->_isCatch != TRUE) {
								v.erase(i);
								++catch_num;
								--current_bugNUm;
								break;
							}
						}
					}
				}
				result = 0;
				wasHandled = true;
				break;
			case WM_DESTROY:
				{
					PostQuitMessage(0);
				}
				DeleteTimerQueue(hTimerQueue);
				result = 1;
				wasHandled = true;
				break;
			}
		}

		if (!wasHandled)
		{
			result = DefWindowProc(hwnd, message, wParam, lParam);
		}
	}

	return result;
}

// Creates a Direct2D bitmap from a resource in the application resource file.
HRESULT DemoApp::LoadBitmapFromResource(ID2D1RenderTarget* pRenderTarget, IWICImagingFactory* pIWICFactory, PCWSTR resourceName, PCWSTR resourceType, UINT destinationWidth, UINT destinationHeight, ID2D1Bitmap** ppBitmap)
{
	IWICBitmapDecoder* pDecoder = NULL;
	IWICBitmapFrameDecode* pSource = NULL;
	IWICStream* pStream = NULL;
	IWICFormatConverter* pConverter = NULL;
	IWICBitmapScaler* pScaler = NULL;

	HRSRC imageResHandle = NULL;
	HGLOBAL imageResDataHandle = NULL;
	void* pImageFile = NULL;
	DWORD imageFileSize = 0;

	// Locate the resource.
	imageResHandle = FindResourceW(HINST_THISCOMPONENT, resourceName, resourceType);
	HRESULT hr = imageResHandle ? S_OK : E_FAIL;
	if (SUCCEEDED(hr))
	{
		// Load the resource.
		imageResDataHandle = LoadResource(HINST_THISCOMPONENT, imageResHandle);
		hr = imageResDataHandle ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		// Lock it to get a system memory pointer.
		pImageFile = LockResource(imageResDataHandle);
		hr = pImageFile ? S_OK : E_FAIL;
	}
	if (SUCCEEDED(hr))
	{
		// Calculate the size.
		imageFileSize = SizeofResource(HINST_THISCOMPONENT, imageResHandle);
		hr = imageFileSize ? S_OK : E_FAIL;

	}
	if (SUCCEEDED(hr))
	{
		// Create a WIC stream to map onto the memory.
		hr = pIWICFactory->CreateStream(&pStream);
	}
	if (SUCCEEDED(hr))
	{
		// Initialize the stream with the memory pointer and size.
		hr = pStream->InitializeFromMemory(reinterpret_cast<BYTE*>(pImageFile), imageFileSize);
	}
	if (SUCCEEDED(hr))
	{
		// Create a decoder for the stream.
		hr = pIWICFactory->CreateDecoderFromStream(pStream, NULL, WICDecodeMetadataCacheOnLoad, &pDecoder);
	}
	if (SUCCEEDED(hr))
	{
		// Create the initial frame.
		hr = pDecoder->GetFrame(0, &pSource);
	}
	if (SUCCEEDED(hr))
	{
		// Convert the image format to 32bppPBGRA (DXGI_FORMAT_B8G8R8A8_UNORM + D2D1_ALPHA_MODE_PREMULTIPLIED).
		hr = pIWICFactory->CreateFormatConverter(&pConverter);
	}
	if (SUCCEEDED(hr))
	{
		// If a new width or height was specified, create an IWICBitmapScaler and use it to resize the image.
		if (destinationWidth != 0 || destinationHeight != 0)
		{
			UINT originalWidth, originalHeight;
			hr = pSource->GetSize(&originalWidth, &originalHeight);
			if (SUCCEEDED(hr))
			{
				if (destinationWidth == 0)
				{
					FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
					destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
				}
				else if (destinationHeight == 0)
				{
					FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
					destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
				}

				hr = pIWICFactory->CreateBitmapScaler(&pScaler);
				if (SUCCEEDED(hr))
				{
					hr = pScaler->Initialize(pSource, destinationWidth, destinationHeight, WICBitmapInterpolationModeCubic);
					if (SUCCEEDED(hr))
					{
						hr = pConverter->Initialize(pScaler, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
					}
				}
			}
		}
		else
		{
			hr = pConverter->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.f, WICBitmapPaletteTypeMedianCut);
		}
	}
	if (SUCCEEDED(hr))
	{
		//create a Direct2D bitmap from the WIC bitmap.
		hr = pRenderTarget->CreateBitmapFromWicBitmap(pConverter, NULL, ppBitmap);
	}

	SAFE_RELEASE(pDecoder);
	SAFE_RELEASE(pSource);
	SAFE_RELEASE(pStream);
	SAFE_RELEASE(pConverter);
	SAFE_RELEASE(pScaler);

	return hr;
}

HRESULT SoundDie(IXAudio2* pXaudio2)
{
	HRESULT hr = S_OK;

	CWaveFile wav;
	if (FAILED(hr = wav.Open(L".\\die.wav", NULL, WAVEFILE_READ)))
	{
		return hr;
	}

	// Get format of wave file
	WAVEFORMATEX* pwfx = wav.GetFormat();

	// Calculate how many bytes and samples are in the wave
	DWORD cbWaveSize = wav.GetSize();

	// Read the sample data into memory
	BYTE* pbWaveData = new BYTE[cbWaveSize];

	if (FAILED(hr = wav.Read(pbWaveData, cbWaveSize, &cbWaveSize)))
	{
		SAFE_DELETE_ARRAY(pbWaveData);
		return hr;
	}

	//
	// Play the wave using a XAudio2SourceVoice
	//

	// Create the source voice
	IXAudio2SourceVoice* pSourceVoice;
	if (FAILED(hr = pXaudio2->CreateSourceVoice(&pSourceVoice, pwfx)))
	{
		SAFE_DELETE_ARRAY(pbWaveData);
		return hr;
	}

	// Submit the wave sample data using an XAUDIO2_BUFFER structure
	XAUDIO2_BUFFER buffer = { 0 };
	buffer.pAudioData = pbWaveData;
	buffer.Flags = XAUDIO2_END_OF_STREAM;  // tell the source voice not to expect any data after this buffer
	buffer.AudioBytes = cbWaveSize;

	if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer)))
	{
		pSourceVoice->DestroyVoice();
		SAFE_DELETE_ARRAY(pbWaveData);
		return hr;
	}
	hr = pSourceVoice->SetVolume(volume);
	hr = pSourceVoice->Start(0);

	return hr;
}

HRESULT SoundWall(IXAudio2* pXaudio2)
{
	HRESULT hr = S_OK;
	CRUSH = TRUE;
	CWaveFile wav;
	if (FAILED(hr = wav.Open(L".\\tack.wav", NULL, WAVEFILE_READ)))
	{
		return hr;
	}

	// Get format of wave file
	WAVEFORMATEX* pwfx = wav.GetFormat();

	// Calculate how many bytes and samples are in the wave
	DWORD cbWaveSize = wav.GetSize();

	// Read the sample data into memory
	BYTE* pbWaveData = new BYTE[cbWaveSize];

	if (FAILED(hr = wav.Read(pbWaveData, cbWaveSize, &cbWaveSize)))
	{
		SAFE_DELETE_ARRAY(pbWaveData);
		return hr;
	}

	//
	// Play the wave using a XAudio2SourceVoice
	//

	// Create the source voice
	IXAudio2SourceVoice* pSourceVoice;
	if (FAILED(hr = pXaudio2->CreateSourceVoice(&pSourceVoice, pwfx)))
	{
		SAFE_DELETE_ARRAY(pbWaveData);
		return hr;
	}

	// Submit the wave sample data using an XAUDIO2_BUFFER structure
	XAUDIO2_BUFFER buffer = { 0 };
	buffer.pAudioData = pbWaveData;
	buffer.Flags = XAUDIO2_END_OF_STREAM;  // tell the source voice not to expect any data after this buffer
	buffer.AudioBytes = cbWaveSize;

	if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer)))
	{
		pSourceVoice->DestroyVoice();
		SAFE_DELETE_ARRAY(pbWaveData);
		return hr;
	}
	hr = pSourceVoice->SetVolume(volume);
	hr = pSourceVoice->Start(0);
	
	return hr;
}




HRESULT SoundClick(IXAudio2* pXaudio2)
{
	HRESULT hr = S_OK;
	CWaveFile wav;
	if (FAILED(hr = wav.Open(L".\\click.wav", NULL, WAVEFILE_READ)))
	{
		return hr;
	}

	// Get format of wave file
	WAVEFORMATEX* pwfx = wav.GetFormat();

	// Calculate how many bytes and samples are in the wave
	DWORD cbWaveSize = wav.GetSize();

	// Read the sample data into memory
	BYTE* pbWaveData = new BYTE[cbWaveSize];

	if (FAILED(hr = wav.Read(pbWaveData, cbWaveSize, &cbWaveSize)))
	{
		SAFE_DELETE_ARRAY(pbWaveData);
		return hr;
	}

	//
	// Play the wave using a XAudio2SourceVoice
	//

	// Create the source voice
	IXAudio2SourceVoice* pSourceVoice;
	if (FAILED(hr = pXaudio2->CreateSourceVoice(&pSourceVoice, pwfx)))
	{
		SAFE_DELETE_ARRAY(pbWaveData);
		return hr;
	}

	// Submit the wave sample data using an XAUDIO2_BUFFER structure
	XAUDIO2_BUFFER buffer = { 0 };
	buffer.pAudioData = pbWaveData;
	buffer.Flags = XAUDIO2_END_OF_STREAM;  // tell the source voice not to expect any data after this buffer
	buffer.AudioBytes = cbWaveSize;

	if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer)))
	{
		pSourceVoice->DestroyVoice();
		SAFE_DELETE_ARRAY(pbWaveData);
		return hr;
	}
	hr = pSourceVoice->SetVolume(volume);
	hr = pSourceVoice->Start(0);

	return hr;
}


HRESULT SoundThrow(IXAudio2* pXaudio2)
{
	HRESULT hr = S_OK;
	CWaveFile wav;
	if (FAILED(hr = wav.Open(L".\\throwing.wav", NULL, WAVEFILE_READ)))
	{
		return hr;
	}

	// Get format of wave file
	WAVEFORMATEX* pwfx = wav.GetFormat();

	// Calculate how many bytes and samples are in the wave
	DWORD cbWaveSize = wav.GetSize();

	// Read the sample data into memory
	BYTE* pbWaveData = new BYTE[cbWaveSize];

	if (FAILED(hr = wav.Read(pbWaveData, cbWaveSize, &cbWaveSize)))
	{
		SAFE_DELETE_ARRAY(pbWaveData);
		return hr;
	}

	//
	// Play the wave using a XAudio2SourceVoice
	//

	// Create the source voice
	IXAudio2SourceVoice* pSourceVoice;
	if (FAILED(hr = pXaudio2->CreateSourceVoice(&pSourceVoice, pwfx)))
	{
		SAFE_DELETE_ARRAY(pbWaveData);
		return hr;
	}

	// Submit the wave sample data using an XAUDIO2_BUFFER structure
	XAUDIO2_BUFFER buffer = { 0 };
	buffer.pAudioData = pbWaveData;
	buffer.Flags = XAUDIO2_END_OF_STREAM;  // tell the source voice not to expect any data after this buffer
	buffer.AudioBytes = cbWaveSize;

	if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer)))
	{
		pSourceVoice->DestroyVoice();
		SAFE_DELETE_ARRAY(pbWaveData);
		return hr;
	}
	hr = pSourceVoice->SetVolume(volume);
	hr = pSourceVoice->Start(0);

	return hr;
}


HRESULT SoundBGM(IXAudio2* pXaudio2)
{
	HRESULT hr = S_OK;
	CWaveFile wav;
	if (FAILED(hr = wav.Open(L".\\BGM.wav", NULL, WAVEFILE_READ)))
	{
		return hr;
	}

	// Get format of wave file
	WAVEFORMATEX* pwfx = wav.GetFormat();

	// Calculate how many bytes and samples are in the wave
	DWORD cbWaveSize = wav.GetSize();

	// Read the sample data into memory
	BYTE* pbWaveData = new BYTE[cbWaveSize];

	if (FAILED(hr = wav.Read(pbWaveData, cbWaveSize, &cbWaveSize)))
	{
		SAFE_DELETE_ARRAY(pbWaveData);
		return hr;
	}

	//
	// Play the wave using a XAudio2SourceVoice
	//

	// Create the source voice
	if (FAILED(hr = pXaudio2->CreateSourceVoice(&pSourceVoice, pwfx)))
	{
		SAFE_DELETE_ARRAY(pbWaveData);
		return hr;
	}

	// Submit the wave sample data using an XAUDIO2_BUFFER structure
	XAUDIO2_BUFFER buffer = { 0 };
	buffer.pAudioData = pbWaveData;
	buffer.Flags = XAUDIO2_END_OF_STREAM;  // tell the source voice not to expect any data after this buffer
	buffer.AudioBytes = cbWaveSize;

	if (FAILED(hr = pSourceVoice->SubmitSourceBuffer(&buffer)))
	{
		pSourceVoice->DestroyVoice();
		SAFE_DELETE_ARRAY(pbWaveData);
		return hr;
	}
	hr = pSourceVoice->SetVolume(bgm_volume);
	if (SOUND)
		hr = pSourceVoice->Start(0);
	return hr;
}