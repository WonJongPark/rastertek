////////////////////////////////////////////////////////////////////////////////
// Filename: light.vs
////////////////////////////////////////////////////////////////////////////////

/////////////
// DEFINES //
/////////////
#define NUM_LIGHTS 4


/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer LightPositionBuffer
{
	float4 lightPosition[NUM_LIGHTS];
};

//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 lightPos[NUM_LIGHTS] : TEXCOORD1;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType LightVertexShader(VertexInputType input)
{
	PixelInputType output;
	float4 worldPosition;
	int i;
	
	
	// 올바른 행렬 계산을 위해 위치 벡터를 4성분으로 만듦.
	input.position.w = 1.0f;
	
	// 월드, 뷰, 투영 행렬에 대해 정점의 위치를 계산.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// 픽셀 셰이더를 위해 텍스처 좌표를 저장.
	output.tex = input.tex;
	
	// 법선 벡터를 월드 행렬에 대해서만 계산.
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	
	// 법선 벡터를 정규화.
	output.normal = normalize(output.normal);
	
	// 월드 공간에서의 정점 위치를 계산.
	worldPosition = mul(input.position, worldMatrix);
	
	for (i = 0; i < NUM_LIGHTS; i++)
	{
		// 각 광원의 위치와 월드 공간 정점 위치를 이용해 광원 방향을 결정.
		output.lightPos[i] = lightPosition[i].xyz - worldPosition.xyz;
		
		// 광원 방향 벡터를 정규화.
		output.lightPos[i] = normalize(output.lightPos[i]);
	}
	
	return output;
}
