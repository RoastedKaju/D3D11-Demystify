// This constant buffer's layout must exactly match Shader::MatrixBufferType.
cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// What comes in per-vertex, laid out exactly as described by polygonLayout[]
// in Shader::InitializeShader.
struct VertexInputType
{
    float4 position : POSITION;
    float4 color : COLOR;
};

// What this shader hands off to the rasterizer -> pixel shader.
struct PixelInputType
{
    float4 position : SV_POSITION;   // SV_POSITION = "this is the clip-space position", required output
    float4 color : COLOR;
};

PixelInputType ColorVertexShader(VertexInputType input)
{
    PixelInputType output;

    // Force w to 1 for a position (as opposed to a direction, which would be 0)
    // before doing the matrix multiplications below.
    input.position.w = 1.0f;

    // Transform the vertex from model space -> world space -> view space -> clip space.
    // Order matters: world first, then view, then projection.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Colour passes through untouched - it'll be interpolated across the
    // triangle's surface automatically before reaching the pixel shader.
    output.color = input.color;

    return output;
}