////////////////////////////////////////////////////////////////////////////////
// Filename: modelclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "modelclass.h"

ModelClass::ModelClass()
{
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_Texture = nullptr;
	m_model = nullptr;
}

ModelClass::ModelClass(const ModelClass& other)
{
}

ModelClass::~ModelClass()
{
}

bool ModelClass::Initialize(
	ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* textureFilename, char* modelFilename)
{
	bool result;

	// 모델 데이터를 읽어 들임.
	result = LoadModel(modelFilename);
	if (!result)
	{
		return false;
	}

	// 정점·인덱스 버퍼를 초기화.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}

	// 이 모델에 사용할 텍스처를 로드.
	result = LoadTexture(device, deviceContext, textureFilename);
	if (!result)
	{
		return false;
	}

	return true;
}

void ModelClass::Shutdown()
{
	// model texture를 해제.
	ReleaseTexture();

	// 정점·인덱스 버퍼를 해제.
	ShutdownBuffers();

	// model data를 해제.
	ReleaseModel();

	return;
}

void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	// 정점·인덱스 버퍼를 그래픽 파이프라인에 올려 그릴 준비.
	RenderBuffers(deviceContext);

	return;
}

int ModelClass::GetIndexCount()
{
	return m_indexCount;
}

ID3D11ShaderResourceView* ModelClass::GetTexture()
{
	return m_Texture->GetTexture();
}

bool ModelClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	// 정점 배열을 생성.
	vertices = new VertexType[m_vertexCount];
	if (!vertices)
	{
		return false;
	}

	// 인덱스 배열 생성.
	indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

	// 정점 배열과 인덱스 배열에 데이터를 채움.
	for (int i = 0; i < m_vertexCount; ++i)
	{
		vertices[i].position = XMFLOAT3(m_model[i].x, m_model[i].y, m_model[i].z);
		vertices[i].texture = XMFLOAT2(m_model[i].tu, m_model[i].tv);
		vertices[i].normal = XMFLOAT3(m_model[i].nx, m_model[i].ny, m_model[i].nz);

		indices[i] = i;
	}

	// 정적 정점 버퍼의 설명 구조체를 설정.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// 서브리소스 구조체에 정점 데이터의 포인터를 저장.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// 정점 버퍼를 생성한다.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// 정점 인덱스 버퍼의 설명 구조체를 설정.
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

	// 인덱스 버퍼를 생성한다.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// 정점·인덱스 버퍼를 생성하고 채웠으니 배열을 해제.
	delete [] vertices;
	vertices = nullptr;

	delete[] indices;
	indices = nullptr;

	return true;
}

void ModelClass::ShutdownBuffers()
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

	return;
}

void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
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

bool ModelClass::LoadTexture(
	ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
	bool result;


	// 텍스처 객체를 생성하고 초기화.
	m_Texture = new TextureClass;

	result = m_Texture->Initialize(device, deviceContext, filename);
	if (!result)
	{
		return false;
	}

	return true;
}

void ModelClass::ReleaseTexture()
{
	// 텍스처 객체를 해제.
	if (m_Texture)
	{
		m_Texture->Shutdown();
		delete m_Texture;
		m_Texture = nullptr;
	}
}

bool ModelClass::LoadModel(char* filename)
{
	ifstream fin;
	char input;


	// 모델 파일을 염.
	fin.open(filename);

	// 파일을 열 수 없으면 종료.
	if (fin.fail())
	{
		return false;
	}

	// 정점 수 값이 나올 때까지 읽음.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}

	// 정점 수를 읽어 들임.
	fin >> m_vertexCount;

	// 인덱스 수를 정점 수와 같게 설정.
	m_indexCount = m_vertexCount;

	// 읽어 들인 정점 수만큼 모델 배열을 생성.
	m_model = new ModelType[m_vertexCount];

	// 데이터 시작 지점까지 읽음.
	fin.get(input);
	while (input != ':')
	{
		fin.get(input);
	}
	fin.get(input);
	fin.get(input);

	// 정점 데이터를 읽어 들임.
	for (int i = 0; i < m_vertexCount; ++i)
	{
		fin >> m_model[i].x >> m_model[i].y >> m_model[i].z;
		fin >> m_model[i].tu >> m_model[i].tv;
		fin >> m_model[i].nx >> m_model[i].ny >> m_model[i].nz;
	}

	// 모델 파일을 닫음.
	fin.close();

	return true;
}

void ModelClass::ReleaseModel()
{
	if (m_model)
	{
		delete[] m_model;
		m_model = nullptr;
	}

	return;
}
