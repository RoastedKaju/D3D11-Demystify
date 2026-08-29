#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

#include "Texture.hpp"

using namespace DirectX;

class QuadMesh
{
public:
	QuadMesh();
	~QuadMesh();

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, const char*, const char*, const char*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();

	ID3D11ShaderResourceView* GetTextureA();
	ID3D11ShaderResourceView* GetTextureB();
	ID3D11ShaderResourceView* GetTextureAlpha();

private:
	struct VertexType
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture;
		DirectX::XMFLOAT3 normal;
	};

	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTextures(ID3D11Device*, ID3D11DeviceContext*, const char*, const char*, const char*);
	void ReleaseTextures();

private:
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;

	int m_vertexCount;
	int m_indexCount;

	Texture* m_textureA;
	Texture* m_textureB;
	Texture* m_textureAlpha;
};