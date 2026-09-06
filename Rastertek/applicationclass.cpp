// applicationclass.cpp

#include "applicationclass.h"


ApplicationClass::ApplicationClass()
{
	m_Direct3D = nullptr;
	m_Camera = nullptr;
	m_Model = nullptr;
	m_LightShader = nullptr;
	m_Light = nullptr;
}


ApplicationClass::ApplicationClass(const ApplicationClass& other)
{
}


ApplicationClass::~ApplicationClass()
{
}


bool ApplicationClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	char textureFilename[128];
	bool result = false;
	 
	// Direct3D 객체를 생성.
	m_Direct3D = new D3DClass;
	if (!m_Direct3D)
	{
		return false;
	}

	// Direct3D object를 초기화.
	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D", L"Error", MB_OK);
		return false;
	}

	// 카메라 객체를 생성.
	m_Camera = new CameraClass;
	if (!m_Camera)
	{
		return false;
	}

	// 카메라의 초기 위치를 설정.
	m_Camera->SetPosition(0.0f, 0.0f, -5.0f);

	// 모델 객체를 생성하고 초기화.
	m_Model = new ModelClass;

	// 불러올 텍스처 파일의 이름을 설정.
	strcpy_s(textureFilename, "data/stone01.tga");

	result = m_Model->Initialize(
		m_Direct3D->GetDevice(), m_Direct3D->GetDeviceContext(), textureFilename);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the model object.", L"Error", MB_OK);
		return false;
	}

	// 라이트 셰이더 객체를 생성하고 초기화.
	m_LightShader = new LightShaderClass;

	result = m_LightShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
		return false;
	}

	// 빛 객체를 생성하고 초기화.
	m_Light = new LightClass;

	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetDirection(0.0f, 0.0f, 1.0f);

	return true;
}


void ApplicationClass::Shutdown()
{
	// 라이트 객체를 해제.
	if (m_Light)
	{
		delete m_Light;
		m_Light = nullptr;
	}

	// 라이트 셰이더 객체를 해제.
	if (m_LightShader)
	{
		m_LightShader->Shutdown();
		delete m_LightShader;
		m_LightShader = nullptr;
	}

	// 모델 객체를 해제.
	if (m_Model)
	{
		m_Model->Shutdown();
		delete m_Model;
		m_Model = nullptr;
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
	static float rotation = 0.0f;
	bool result = false;

	// 삼각형 회전값을 매 프레임 갱신.
	rotation -= 0.0174532925f * 0.1f;
	if (rotation < 0.0f)
	{
		// 같은 방향을 유지하면서 회전각을 양수로 돌려놓음.
		rotation += XM_2PI;
	}

	// 그래픽 씬 렌더링
	result = Render(rotation);
	if (!result)
	{
		return false;
	}

	return true;
}


bool ApplicationClass::Render(float rotation)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;


	// 장면을 시작하기 위해 버퍼를 지움.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// 카메라의 위치를 기반으로 뷰 행렬을 생성.
	m_Camera->Render();

	// 카메라와 d3d 객체로부터 월드·뷰·투영 행렬을 얻음.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	// 삼각형이 회전하도록 월드 행렬을 회전 값으로 회전시킴.
	worldMatrix = XMMatrixRotationY(rotation);

	// 모델의 정점·인덱스 버퍼를 그래픽 파이프라인에 올려 그릴 준비를 함.
	m_Model->Render(m_Direct3D->GetDeviceContext());

	// 라이트 셰이더를 사용해 모델을 렌더링.
	result = m_LightShader->Render(
		m_Direct3D->GetDeviceContext(),
		m_Model->GetIndexCount(),
		worldMatrix,
		viewMatrix,
		projectionMatrix,
		m_Model->GetTexture(),
		m_Light->GetDirection(),
		m_Light->GetDiffuseColor()
	);
	if (!result)
	{
		return false;
	}

	// 화면에 렌더링된 씬 표현
	m_Direct3D->EndScene();

	return true;
}
