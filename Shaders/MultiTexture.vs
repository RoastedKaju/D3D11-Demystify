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

PixelInputType MultiTextureVertexShader(VertexInputType input)
{
    PixelInputType output;

    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Both textures are sampled with this SAME set of UVs - the simplest
    // form of multitexturing. (A more elaborate version could carry a
    // second, independent UV set per texture - useful for things like a
    // detail texture tiled at a different scale than the base map - but
    // that's a bigger change to the vertex format; this tutorial is about
    // the blending idea itself.)
    output.tex = input.tex;

    return output;
}
