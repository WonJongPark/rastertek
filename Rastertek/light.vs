////////////////////////////////////////////////////////////////////////////////
// Filename: light.vs
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer CameraBuffer
{
    float3 cameraPosition;
    float padding;
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
    float3 viewDirection : TEXCOORD1;
};


////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;
    float4 worldPosition;
    
    
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
    
    // 월드 공간의 정점에서 카메라를 향하는 시선 벡터를 계산.
    output.viewDirection = cameraPosition.xyz - worldPosition.xyz;
    
    // 시선 방향 벡터를 정규화.
    output.viewDirection = normalize(output.viewDirection);
    
    return output;
}
