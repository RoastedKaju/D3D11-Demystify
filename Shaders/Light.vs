cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// 2nd constant buffer, bound to a different slot (b1)
// Matrix buffer is (b0)
cbuffer CameraBuffer
{
    float3 cameraPosition;
    float  padding;
};

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
    float3 viewDirection : VIEWDIR; // new - direction from this point back to camera
};

PixelInputType LightVertexShader(VertexInputType input)
{
    PixelInputType output;
    float4 worldPosition;

    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.tex = input.tex;

    output.normal = mul(input.normal, (float3x3)worldMatrix);
    output.normal = normalize(output.normal);

    // We need this vertex's position in WORLD space (not clip space) to
    // compute a direction back to the camera - clip-space position isn't
    // usable for this, since it's already been projected/distorted by the
    // projection matrix.
    worldPosition = mul(input.position, worldMatrix);

    // View direction: from this point on the surface, back toward the
    // camera. This is what the pixel shader compares the reflected light
    // ray against to determine specular highlight strength.
    output.viewDirection = cameraPosition.xyz - worldPosition.xyz;
    output.viewDirection = normalize(output.viewDirection);

    return output;
}
