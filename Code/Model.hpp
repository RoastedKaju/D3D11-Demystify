#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

#include "Texture.hpp"

using namespace DirectX;

class Model
{
public:
	Model();
	~Model();

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, const char*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

private:
	// This must match the input layout the shader expects
	// See (Shader::InitializeShader)
	struct VertexType
	{
		XMFLOAT3 position;
		//XMFLOAT4 color;
		XMFLOAT2 texture;
	};

	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTexture(ID3D11Device*, ID3D11DeviceContext*, const char*);
	void ReleaseTexture();

private:
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;
	int m_vertexCount;
	int m_indexCount;
	Texture* m_texture;
};