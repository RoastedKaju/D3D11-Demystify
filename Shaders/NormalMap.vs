cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
};

PixelInputType NormalMapVertexShader(VertexInputType input)
{
    PixelInputType output;

    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.tex = input.tex;

    // Normal, tangent, and binormal are all directions, not points - same
    // 3x3-only rotation (never the full 4x4, which would incorrectly
    // apply translation) used for normals in every lighting vertex shader
    // in this project, just done three times now instead of once.
    output.normal = normalize(mul(input.normal, (float3x3)worldMatrix));
    output.tangent = normalize(mul(input.tangent, (float3x3)worldMatrix));
    output.binormal = normalize(mul(input.binormal, (float3x3)worldMatrix));

    return output;
}
