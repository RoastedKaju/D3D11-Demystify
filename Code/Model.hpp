#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

using namespace DirectX;

class Model
{
public:
	Model();
	~Model();

	bool Initialize(ID3D11Device*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();

private:
	// This must match the input layout the shader expects
	// See (Shader::InitializeShader)
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

private:
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;
	int m_vertexCount;
	int m_indexCount;
};