#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <fstream>

using namespace DirectX;

// Must match #define NUM_LIGHTS in shader exactly
// There is no automatic sharing between HLSL and C++ so all three copies have to be kept in Sync.
constexpr int NUM_LIGHTS = 4;

// Light related buffers are arrays
// Positions need to go to vertex shader
// Colors go to pixel shader
class MultiLightShader
{
public:
	MultiLightShader();
	~MultiLightShader();

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, const XMFLOAT3*, const XMFLOAT4*);

private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	// Each entry is float 4 for 16 byte boundary purposes
	// This is for vertex shader
	struct LightPositionBufferType
	{
		XMFLOAT4 positions[NUM_LIGHTS];
	};


	struct LightColorBufferType
	{
		XMFLOAT4 colors[NUM_LIGHTS];
	};

	bool InitializeShader(ID3D11Device*, HWND, const char*, const char*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3DBlob*, HWND, const char*);

	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, const XMFLOAT3*, const XMFLOAT4*);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_lightPositionBuffer;
	ID3D11Buffer* m_lightColorBuffer;

};