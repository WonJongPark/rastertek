////////////////////////////////////////////////////////////////////////////////
// Filename: bitmapclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "bitmapclass.h"

BitmapClass::BitmapClass()
{
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_Texture = nullptr;
	m_vertexCount = m_indexCount = 6;
	m_screenWidth = m_screenHeight = 0;
	m_bitmapWidth = m_bitmapHeight = 0;
	m_renderX = m_renderY = 0;
	m_prevPosX = m_prevPosY = -1;
}

BitmapClass::BitmapClass(const BitmapClass&)
{
}

BitmapClass::~BitmapClass()
{
}

bool BitmapClass::Initialize(
	ID3D11Device* device,
	ID3D11DeviceContext* deviceContext,
	int screenWidth,
	int screenHeight,
	char* textureFilename,
	int renderX,
	int renderY)
{
	// 화면 크기를 저장.
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;

	// 비트맵을 렌더랑할 위치를 저장.
	SetRenderLocation(renderX, renderY);

	// 비트맵 사각형의 지오메트리를 담을 정점·인덱스 버퍼를 초기화.
	if (!InitializerBuffers(device))
	{
		return false;
	}

	// 이 비트맵에 쓸 텍스처를 부러옴.
	return LoadTexture(device, deviceContext, textureFilename);
}

void BitmapClass::Shutdown()
{
	// 비트맵 텍스처를 해제.
	ReleaseTexture();
	// 정점·인덱스 버퍼를 해제.
	ShutdownBuffers();

	return;
}

bool BitmapClass::Render(ID3D11DeviceContext* deviceContext)
{
	// 비트맵 위치가 이전 위치에서 바뀌었으면 버퍼를 갱신.
	if (!UpdateBuffers(deviceContext))
	{
		return false;
	}
	
	// 정점·인덱스 버퍼를 그래픽 파이프라인에 올려 그릴 준비를 함.
	RenderBuffers(deviceContext);
	return true;
}

int BitmapClass::GetIndexCount()
{
	return m_indexCount;
}

ID3D11ShaderResourceView* BitmapClass::GetTexture()
{
	return m_Texture->GetTexture();
}

void BitmapClass::SetRenderLocation(int x, int y)
{
	m_renderX = x;
	m_renderY = y;
	return;
}

bool BitmapClass::InitializerBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i;


	// 이전 렌더링 위치를 -1로 초기화.
	m_prevPosX = -1;
	m_prevPosY = -1;

	// 정점 배열의 정점 개수를 설정.
	m_vertexCount = 6;

	// 인덱스 배열의 인덱스 개수를 설정.
	m_indexCount = m_vertexCount;

	// 정점 배열을 생성.
	vertices = new VertexType[m_vertexCount];

	// 인덱스 배열을 생성.
	indices = new unsigned long[m_indexCount];

	// 먼저 정점 배열을 0으로 초기화.
	memset(vertices, 0, (sizeof(VertexType) * m_vertexCount));

	// 인덱스 배열에 데이터를 채움.
	for (i = 0; i < m_indexCount; i++)
	{
		indices[i] = i;
	}

	// 동적 정점 버퍼의 description을 설정.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// 서브리소스 구조체에 정점 데이터의 포인터를 지정.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// 이제 마지막으로 정점 버퍼를 생성.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// 인덱스 버퍼의 설명 구조체를 설정.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// 서브리소스 구조체에 인덱스 데이터의 포인터를 지정.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// 인덱스 버퍼를 생성.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// 정점·인덱스 버퍼를 생성하고 채웠으니 배열을 해제.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}

void BitmapClass::ShutdownBuffers()
{
	// 인덱스 버퍼를 해제.
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = nullptr;
	}

	// 정점 버퍼를 해제.
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}
}

bool BitmapClass::UpdateBuffers(ID3D11DeviceContext* deviceContext)
{
	float left, right, top, bottom;
	VertexType* vertices;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* dataPtr;
	HRESULT result;


	// 이 비트맵을 렌더링할 위치가 바뀌지 않았다면 정점 버퍼를 갱신하지 않음.
	if ((m_prevPosX == m_renderX) && (m_prevPosY == m_renderY))
	{
		return true;
	}

	// 렌더링 위치가 바뀌었다면 새 위치를 저장하고 정점 버퍼를 갱신.
	m_prevPosX = m_renderX;
	m_prevPosY = m_renderY;

	// 정점 배열을 생성한다.
	vertices = new VertexType[m_vertexCount];

	// 비트맵 왼쪽 변의 화면 좌표를 계산.
	left = (float)((m_screenWidth / 2) * -1) + (float)m_renderX;

	// 비트맵 오른쪽 변의 화면 좌표를 계산.
	right = left + (float)m_bitmapWidth;

	// 비트맵 위쪽 변의 화면 좌표를 계산.
	top = (float)(m_screenHeight / 2) - (float)m_renderY;

	// 비트맵 아래쪽 변의 화면 좌표를 계산.
	bottom = top - (float)m_bitmapHeight;

	// 정점 배열에 데이터를 채움.
	// 첫 번째 삼각형.
	vertices[0].position = XMFLOAT3(left, top, 0.0f);  // 좌상단.
	vertices[0].texture = XMFLOAT2(0.0f, 0.0f);

	vertices[1].position = XMFLOAT3(right, bottom, 0.0f);  // 우하단.
	vertices[1].texture = XMFLOAT2(1.0f, 1.0f);

	vertices[2].position = XMFLOAT3(left, bottom, 0.0f);  // 좌하단.
	vertices[2].texture = XMFLOAT2(0.0f, 1.0f);

	// 두 번째 삼각형.
	vertices[3].position = XMFLOAT3(left, top, 0.0f);  // 좌상단.
	vertices[3].texture = XMFLOAT2(0.0f, 0.0f);

	vertices[4].position = XMFLOAT3(right, top, 0.0f);  // 우상단.
	vertices[4].texture = XMFLOAT2(1.0f, 0.0f);

	vertices[5].position = XMFLOAT3(right, bottom, 0.0f);  // 우하단.
	vertices[5].texture = XMFLOAT2(1.0f, 1.0f);

	// 정점 버퍼를 매핑.
	result = deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// 상수 버퍼 내부 데이터의 포인터를 얻음.
	dataPtr = (VertexType*)mappedResource.pData;

	// 데이터를 정점 버퍼에 복사.
	memcpy(dataPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

	// 정점 버퍼의 잠금을 해제.
	deviceContext->Unmap(m_vertexBuffer, 0);

	// 포인터 참조를 해제.
	dataPtr = 0;

	// 더 이상 필요 없으므로 정점 배열을 해제.
	delete[] vertices;
	vertices = 0;

	return true;
}

void BitmapClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	// 정점 버퍼의 스트라이드와 오프셋을 설정.
	stride = sizeof(VertexType);
	offset = 0;

	// 렌더링할 수 있도록 입력 어셈블러에서 정점 버퍼를 활성화.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// 렌더링할 수 있도록 입력 어셈블러에서 인덱스 버퍼를 활성화.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// 이 정점 버퍼로 그릴 프리미티브 종류를 설정(여기서는 삼각형).
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

bool BitmapClass::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
	bool result;


	// 텍스처 객체를 생성하고 초기화.
	m_Texture = new TextureClass;

	result = m_Texture->Initialize(device, deviceContext, filename);
	if (!result)
	{
		return false;
	}

	// 이 비트맵을 렌더링할 픽셀 크기를 저장.
	m_bitmapWidth = m_Texture->GetWidth();
	m_bitmapHeight = m_Texture->GetHeight();

	return true;
}

void BitmapClass::ReleaseTexture()
{
	// 텍스쳐 객제를 해제.
	if (m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = nullptr;
	}
}
