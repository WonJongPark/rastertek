////////////////////////////////////////////////////////////////////////////////
// Filename: textureclass.cpp
////////////////////////////////////////////////////////////////////////////////

#include "textureclass.h"


TextureClass::TextureClass()
{
	m_targaData = nullptr;
	m_texture = nullptr;
	m_textureView = nullptr;
}

TextureClass::TextureClass(const TextureClass& other)
{
}

TextureClass::~TextureClass()
{
}

bool TextureClass::Initialize(
	ID3D11Device* device, ID3D11DeviceContext* deviceContext, char* filename)
{
	bool result;
	D3D11_TEXTURE2D_DESC textureDesc;
	HRESULT hResult;
	unsigned int rowPitch;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;


	// Targa 이미지 데이터를 메모리로 불러옴.
	result = LoadTarga32Bit(filename);
	if (!result)
	{
		return false;
	}

	// 텍스처의 설명 구조체를 설정.
	textureDesc.Height = m_height;
	textureDesc.Width = m_width;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	// 빈 텍스처를 생성.
	hResult = device->CreateTexture2D(&textureDesc, NULL, &m_texture);
	if (FAILED(hResult))
	{
		return false;
	}

	// targa 이미지 데이터의 행 피치(row pitch)를 설정.
	rowPitch = (m_width * 4) * sizeof(unsigned char);

	// targa 이미지 데이터를 텍스처에 복사.
	deviceContext->UpdateSubresource(m_texture, 0, NULL, m_targaData, rowPitch, 0);

	// 셰이더 리소스 뷰 설명 구조체를 설정.
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	// 텍스처에 대한 셰이더 리소스 뷰를 생성.
	hResult = device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView);
	if (FAILED(hResult))
	{
		return false;
	}

	// 이 텍스처의 밉맵을 생성.
	deviceContext->GenerateMips(m_textureView);

	// 이미지 데이터를 텍스처에 실었으니 Targa 이미지 데이터를 해제.
	delete[] m_targaData;
	m_targaData = nullptr;

	return true;
}

void TextureClass::Shutdown()
{
	// 텍스처 뷰 리소스를 해제.
	if (m_textureView)
	{
		m_textureView->Release();
		m_textureView = nullptr;
	}

	// 텍스처를 해제.
	if (m_texture)
	{
		m_texture->Release();
		m_texture = nullptr;
	}

	// Targa 데이터를 해제.
	if (m_targaData)
	{
		delete[] m_targaData;
		m_targaData = nullptr;
	}

	return;
}

ID3D11ShaderResourceView* TextureClass::GetTexture()
{
	return m_textureView;
}

bool TextureClass::LoadTarga32Bit(char* filename)
{
	int error, bpp, imageSize, index, k;
	FILE* filePtr;
	unsigned int count;
	TargaHeader targaFileHeader;
	unsigned char* targaImage;


	// Targa 파일을 바이너리 읽기 모드로 염.
	error = fopen_s(&filePtr, filename, "rb");
	if (error != 0)
	{
		return false;
	}

	// 파일 헤더를 읽어 들임.
	count = (unsigned int)fread(&targaFileHeader, sizeof(TargaHeader), 1, filePtr);
	if (count != 1)
	{
		return false;
	}

	// 헤더에서 중요한 정보를 얻음.
	m_height = (int)targaFileHeader.height;
	m_width = (int)targaFileHeader.width;
	bpp = (int)targaFileHeader.bpp;

	// 24비트가 아니라 32비트 인지 확인.
	if (bpp != 32)
	{
		return false;
	}

	// 32비트 이미지 데이터의 크기를 계산.
	imageSize = m_width * m_height * 4;

	// Targa 이미지 데이터를 담을 메모리를 할당.
	targaImage = new unsigned char[imageSize];

	// Targa 이미지 데이터를 읽어 들임.
	count = (unsigned int)fread(targaImage, 1, imageSize, filePtr);
	if (count != imageSize)
	{
		return false;
	}

	// 파일을 닫음.
	error = fclose(filePtr);
	if (error != 0)
	{
		return false;
	}

	// Targa 대상 데이터를 담을 메모리를 할당.
	m_targaData = new unsigned char[imageSize];

	// Targa 대상 데이터를 배열의 인덱스를 초기화.
	index = 0;

	// Targa 이미디 데이터의 인덱스를 초기화.
	k = (m_width * m_height * 4) - (m_width * 4);

	// Targa 포맷은 상하가 뒤집혀 저장되고 RGBA 순서도 아니므로,
	// Targa 이미지 데이터를 올바른 순서로 대상 배열에 복사.
	for (int i = 0; i < m_height; ++i)
	{
		for (int j = 0; j < m_width; ++j)
		{
			m_targaData[index + 0] = targaImage[k + 2];	// 빨강.
			m_targaData[index + 1] = targaImage[k + 1];	// 초록.
			m_targaData[index + 2] = targaImage[k + 0];	// 파랑.
			m_targaData[index + 3] = targaImage[k + 3];	// 알파.

			// targa 데이터의 인덱스를 증가.
			k += 4;
			index += 4;
		}

		// 상하가 뒤집혀 읽히므로 targa 이미지 데이터 인덱스를 이전 행의 열 시작 위치로 되돌림.
		k -= (m_width * 8);
	}

	// 대상 배열로 복사했으니 Targa 이미지 데이터를 해제.
	delete[] targaImage;
	targaImage = nullptr;

	return true;
}

int TextureClass::GetWidth()
{
	return m_width;
}

int TextureClass::GetHeight()
{
	return m_height;
}
