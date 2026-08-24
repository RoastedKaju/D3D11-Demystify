#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>

#include "Texture.hpp"

using namespace DirectX;

/**
* Only meant to be used with multi-texture shader,
* this is just an example triangle.
*/
class MultiTextureTriangle
{
public:
	MultiTextureTriangle();
	~MultiTextureTriangle() = default;

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, const char*, const char*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();

	ID3D11ShaderResourceView* GetTextureA();
	ID3D11ShaderResourceView* GetTextureB();

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

	bool LoadTextures(ID3D11Device*, ID3D11DeviceContext*, const char*, const char*);
	void ReleaseTextures();

private:
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;

	int m_vertexCount;
	int m_indexCount;

	Texture* m_textureA;
	Texture* m_textureB;
};