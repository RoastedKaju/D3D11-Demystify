// Must match NUM_LIGHTS in multilight.ps and MultiLightShaderClass exactly -
// HLSL and C++ don't share #defines, so all three have to be kept in sync
// by hand whenever this changes.
#define NUM_LIGHTS 4

cbuffer MatrixBuffer
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

// Bound to slot b1 - one world-space position per light. Point lights have
// no single "direction" the way a directional light does; direction has to
// be computed per-surface-point, per-light, which is why this needs to
// live here instead of being a plain XMFLOAT3 like Tutorial 6's light did.
cbuffer LightPositionBuffer
{
    float4 lightPosition[NUM_LIGHTS];
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
    // An array field in an HLSL semantic-tagged struct automatically
    // expands across consecutive semantic indices - this one line becomes
    // TEXCOORD1, TEXCOORD2, TEXCOORD3, TEXCOORD4 for NUM_LIGHTS = 4,
    // one interpolated direction vector per light.
    float3 lightDir[NUM_LIGHTS] : TEXCOORD1;
};

PixelInputType MultiLightVertexShader(VertexInputType input)
{
    PixelInputType output;
    float4 worldPosition;
    int i;

    input.position.w = 1.0f;

    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    output.tex = input.tex;

    output.normal = mul(input.normal, (float3x3)worldMatrix);
    output.normal = normalize(output.normal);

    // Need this vertex's actual world-space position (not clip-space) to
    // work out the direction toward each light's world-space position.
    worldPosition = mul(input.position, worldMatrix);

    // Unlike a directional light (one fixed direction everywhere), each
    // point light has its own direction from THIS specific vertex - a
    // vertex near light 0 and a vertex near light 3 will get very
    // different lightDir[0] values from each other.
    for (i = 0; i < NUM_LIGHTS; i++)
    {
        output.lightDir[i] = lightPosition[i].xyz - worldPosition.xyz;
        output.lightDir[i] = normalize(output.lightDir[i]);
    }

    return output;
}
