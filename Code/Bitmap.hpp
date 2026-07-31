#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

#include "Texture.hpp"

using namespace DirectX;

// A textured quad positioned in screen (pixel) coordinates, meant to be
// drawn with orthographic projection matrix instead of perspective one.
class Bitmap
{
public:
	Bitmap();
	~Bitmap();

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, int, int, const char*, int, int);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, int);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

public:
	struct Vertex2DType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	bool UpdateBuffers(ID3D11DeviceContext*, int, int);
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTexture(ID3D11Device*, ID3D11DeviceContext*, const char*);
	void ReleaseTexture();

private:
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;
	int m_vertexCount;
	int m_indexCount;
	Texture* m_texture;

	int m_screenWidth;
	int m_screenHeight;

	int m_bitmapWidth;
	int m_bitmapHeight;

	int m_previousPosX;
	int m_previousPosY;
};