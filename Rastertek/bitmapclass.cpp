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
	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	SetRenderLocation(renderX, renderY);

	if (!InitializerBuffers(device))
	{
		return false;
	}

	return LoadTexture(device, deviceContext, textureFilename);
}

void BitmapClass::Shutdown()
{
	ReleaseTexture();
	ShutdownBuffers();
}

bool BitmapClass::Render(ID3D11DeviceContext* deviceContext)
{
	if (!UpdateBuffers(deviceContext))
	{
		return false;
	}
	
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
}

bool BitmapClass::InitializerBuffers(ID3D11Device* device)
{
	VertexType vertices[6] = {};
	unsigned long indices[6] = { 0, 1, 2, 3, 4 ,5 };

	D3D11_BUFFER_DESC vertexDesc = {};
	vertexDesc.ByteWidth = sizeof(vertices);
	vertexDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA vertexData = {};
	vertexData.pSysMem = vertices;

	HRESULT result = device->CreateBuffer(&vertexDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	D3D11_BUFFER_DESC indexDesc = {};
	indexDesc.ByteWidth = sizeof(indices);
	indexDesc.Usage = D3D11_USAGE_DEFAULT;
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices;

	result = device->CreateBuffer(&indexDesc, &indexData, &m_indexBuffer);
	return SUCCEEDED(result);
}

void BitmapClass::ShutdownBuffers()
{
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = nullptr;
	}

	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}
}

bool BitmapClass::UpdateBuffers(ID3D11DeviceContext* deviceContext)
{
	// 위치가 같으면 기존 정점 데이터를 재사용함.
	if (m_prevPosX == m_renderX && m_prevPosY == m_renderY)
	{
		return true;
	}
	
	// 좌상단 기준 픽셀 위치를 중앙 기준 좌표로 변환함.
	float left = m_renderX - m_screenWidth * 0.5f;
	float top = m_screenHeight * 0.5f - m_renderY;
	float right = left + m_bitmapWidth;
	float bottom = top - m_bitmapHeight;

	VertexType vertices[6] =
	{
		{ XMFLOAT3(left, top, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(right, bottom, 0.0f), XMFLOAT2(1.0f, 1.0f) },
		{ XMFLOAT3(left, bottom, 0.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT3(left, top, 0.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT3(right, top, 0.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT3(right, bottom, 0.0f), XMFLOAT2(1.0f, 1.0f) }
	};

	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	HRESULT result = deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	memcpy(mappedResource.pData, vertices, sizeof(vertices));
	deviceContext->Unmap(m_vertexBuffer, 0);

	// 버퍼 갱신에 성공한 위치를 저장함.
	m_prevPosX = m_renderX;
	m_prevPosY = m_renderY;
	return true;
}

void BitmapClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool BitmapClass::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
	m_Texture = new TextureClass;
	if (!m_Texture->Initialize(device, deviceContext, filename))
	{
		return false;
	}

	m_bitmapWidth = m_Texture->GetWidth();
	m_bitmapHeight = m_Texture->GetHeight();
	return true;
}

void BitmapClass::ReleaseTexture()
{
	if (m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = nullptr;
	}
}
