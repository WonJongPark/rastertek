// applicationclass.cpp

#include "applicationclass.h"


ApplicationClass::ApplicationClass()
{
	m_Direct3D = nullptr;
	m_Camera = nullptr;
	m_Model = nullptr;
	m_TextureShader = nullptr;
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

	// 텍스처 셰이더 객체를 생성하고 초기화.
	m_TextureShader = new TextureShaderClass;

	result = m_TextureShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}

	return true;
}


void ApplicationClass::Shutdown()
{

	// 텍스처 셰이더 객체를 해제.
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = nullptr;
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
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;


	// 장면을 시작하기 위해 버퍼를 지움.
	m_Direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// 카메라의 위치를 기반으로 뷰 행렬을 생성.
	m_Camera->Render();

	// 카메라와 d3d 객체로부터 월드·뷰·튜영 행렬을 얻음.
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);

	// 모델의 정점·인덱스 버퍼를 그래픽 파이프라인에 올려 그릴 준비를 함.
	m_Model->Render(m_Direct3D->GetDeviceContext());

	// 텍스처 셰이더를 사용해 모델을 렌더링.
	result = m_TextureShader->Render(
		m_Direct3D->GetDeviceContext(),
		m_Model->GetIndexCount(),
		worldMatrix,
		viewMatrix,
		projectionMatrix,
		m_Model->GetTexture()
	);
	if (!result)
	{
		return false;
	}

	// 화면에 랜더링된 씬 표현
	m_Direct3D->EndScene();

	return true;
}
