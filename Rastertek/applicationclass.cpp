// applicationclass.cpp

#include "applicationclass.h"


ApplicationClass::ApplicationClass()
{
	m_Direct3D = nullptr;
}


ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
}


ApplicationClass::~ApplicationClass()
{
}


bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result = false;
	 
	// Direct3D 객체를 생성하고 초기화
	m_Direct3D = new D3DClass;

	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	return true;
}


void ApplicationClass::Shutdown()
{
	// DirectD3D 객체 해제
	if (m_Direct3D)
	{
		m_Direct3D->Shutdown();
		delete m_Direct3D;
		m_Direct3D = nullptr;
	}

	return;
}


bool ApplicationClass::Frame()
{
	bool result = false;

	// 그래픽씬 랜더링
	result = Render();
	if (!result)
	{
		return false;
	}

	return true;
}


bool ApplicationClass::Render()
{
	// 화면을 그리기 위해 버퍼를 클리어
	m_Direct3D->BeginScene(0.5f, 0.5f, 0.5f, 1.0f);

	// 화면에 랜더링된 씬 표현
	m_Direct3D->EndScene();

	return true;
}