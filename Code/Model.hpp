#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

#include "Texture.hpp"

using namespace DirectX;

class Model
{
public:
	Model();
	~Model();

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, const char*, const char*);
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
		XMFLOAT2 texture;
		XMFLOAT3 normal;
		XMFLOAT3 tangent;
		XMFLOAT3 binormal;
	};

	bool InitializeBuffers(ID3D11Device*, const char*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTexture(ID3D11Device*, ID3D11DeviceContext*, const char*);
	void ReleaseTexture();

	bool LoadModel(const char*);

private:
	static void CalculateTangentBinormal(VertexType& v0, VertexType& v1, VertexType& v2);

	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;
	int m_vertexCount;
	int m_indexCount;
	Texture* m_texture;

	std::vector<VertexType> m_vertices;
	std::vector<unsigned long> m_indices;
};