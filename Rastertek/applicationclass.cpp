// applicationclass.cpp

#include "applicationclass.h"


ApplicationClass::ApplicationClass()
{
	m_Direct3D = nullptr;
	m_Camera = nullptr;
	m_TextureShader = nullptr;
	m_Sprite = nullptr;
	m_Timer = nullptr;
}


ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
}


ApplicationClass::~ApplicationClass()
{
}


bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	char spriteFilename[128];
	bool result;
	
	m_Direct3D = new D3DClass;
	if (!m_Direct3D->Initialize(
		screenWidth, screenHeight, VSYNC_ENABLED, hwnd,
		FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR))
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	// 중앙 기준 2D 좌표에 맞춰 고정 카메라를 설정함.
	m_Camera = new CameraClass;
	m_Camera->SetPosition(0.0f, 0.0f, -10.0f);
	m_Camera->Render();

	m_TextureShader = new TextureShaderClass;
	if (!m_TextureShader->Initialize(m_Direct3D->GetDevice(), hwnd))
	{
		MessageBox(hwnd, L"Could not initializer the texture shader.", L"Error", MB_OK);
		return false;
	}

	// sprite info file we will be using을 설정.
	strcpy_s(spriteFilename, "data/sprite_data_01.txt");
	
	// sprite 객체를 생성하고 초기화.
	m_Sprite = new SpriteClass;

	result = m_Sprite->Initialize(
		m_Direct3D->GetDevice(),
		m_Direct3D->GetDeviceContext(),
		screenWidth,
		screenHeight,
		spriteFilename,
		50,
		50
	);
	if (!result)
	{
		return false;
	}

	// timer 객체를 생성하고 초기화.
	m_Timer = new TimerClass;

	result = m_Timer->Initialize();
	if (!result)
	{
		return false;
	}

	return true;
}


void ApplicationClass::Shutdown()
{
	// 타이머 객체를 해제.
	if (m_Timer)
	{
		delete m_Timer;
		m_Timer = nullptr;
	}

	// 스프라이트 객체를 해제.
	if (m_Sprite)
	{
		m_Sprite->Shutdown();
		delete m_Sprite;
		m_Sprite = nullptr;
	}

	// 텍스처 셰이더와 관련 자원을 해제.
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = nullptr;
	}

	// 카메라 객체를 해제.
	if (m_Camera)
	{
		delete m_Camera;
		m_Camera = nullptr;
	}

	// Direct3D 객체 해제
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
	float frameTime;
	bool result;

	// 시스템 통계를 갱신.
	m_Timer->Frame();

	// 현재 프레임 시간을 가져옴.
	frameTime = m_Timer->GetTime();

	// 프레임 시간을 사용해 스프라이트 객체를 갱신.
	m_Sprite->Update(frameTime);

	// 그래픽 장면을 렌더링.
	result = Render();
	if (!result)
	{
		return false;
	}

	return true;
}


bool ApplicationClass::Render()
{
	XMMATRIX worldMatrix, viewMatrix, orthoMatrix;
	bool result;

	// 장면을 시작하기 위해 버퍼를 지움.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// 카메라와 d3d 객체에서 월드, 뷰, 투영 행렬을 가져옴.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetOrthoMatrix(orthoMatrix);

	// 모든 2D 렌더링을 시작하기 위해 Z 버퍼를 끔.
	m_Direct3D->TurnZBufferOff();

	// 스프라이트 정점·인덱스 버퍼를 그래픽 파이르라인에 올려 그리기를 준비.
	result = m_Sprite->Render(m_Direct3D->GetDeviceContext());
	if (!result)
	{
		return false;
	}

	// 텍스처 셰이더로 스프라이트를 렌더링.
	result = m_TextureShader->Render(
		m_Direct3D->GetDeviceContext(),
		m_Sprite->GetIndexCount(),
		worldMatrix,
		viewMatrix,
		orthoMatrix,
		m_Sprite->GetTexture()
	);	
	if (!result)
	{
		return false;
	}

	// 모든 2D 렌더링이 끝났으니 Z 버퍼를 다시 켬.
	m_Direct3D->TurnZBufferOn();

	// 렌더링된 장면을 화면에 표시.
	m_Direct3D->EndScene();

	return true;
}
