#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <fstream>

using namespace DirectX;

class LightShader
{
public:
	LightShader();
	~LightShader();

	LightShader(const LightShader&) = delete;
	LightShader(LightShader&&) = delete;

	bool Initialize(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, XMFLOAT4, XMFLOAT4, XMFLOAT4, XMFLOAT4, float, XMFLOAT3);

private:
	struct alignas(16) MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

	// Minor C-Buffer in Light.ps
	struct alignas(16) LightBufferType
	{
		XMFLOAT4 diffuseColor;
		XMFLOAT4 ambientColor;
		XMFLOAT4 lightDirection; // Direction needs only 3 floats but for padding reasons we pass it as float 4
		XMFLOAT4 specularColor;
		float specularPower;
		XMFLOAT3 padding;
	};

	struct alignas(16) CameraBufferType
	{
		XMFLOAT3 cameraPosition;
		float padding;
	};

	bool InitializeShader(ID3D11Device*, HWND, const char*, const char*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3DBlob*, HWND, const char*);

	bool SetShaderParameters(ID3D11DeviceContext*, XMMATRIX, XMMATRIX, XMMATRIX, ID3D11ShaderResourceView*, XMFLOAT4, XMFLOAT4, XMFLOAT4, XMFLOAT4, float, XMFLOAT3);
	void RenderShader(ID3D11DeviceContext*, int);

private:
	ID3D11VertexShader* m_vertexShader;
	ID3D11PixelShader* m_pixelShader;
	ID3D11InputLayout* m_layout;
	ID3D11Buffer* m_matrixBuffer;
	ID3D11SamplerState* m_sampleState;
	ID3D11Buffer* m_lightBuffer;
	ID3D11Buffer* m_cameraBuffer;
};