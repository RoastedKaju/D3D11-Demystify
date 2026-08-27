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
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};

PixelInputType VertexShaderEntry(VertexInputType input)
{
    PixelInputType output;

    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Same shared UV set drives both samples. A light map is normally
    // authored specifically for this model's existing UV layout (baked to
    // match it exactly), so there's no need for a second UV channel the
    // way there might be for, say, a tiled detail texture.
    output.tex = input.tex;

    return output;
}
