#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <string>

#include "Texture.hpp"

using namespace DirectX;

class Sprite
{
public:
	Sprite();
	~Sprite();

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, int, int, const std::vector<std::string>&, float, int, int);
	void Shutdown();

	bool Render(ID3D11DeviceContext*, int, int, float);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();

private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

	bool InitializeBuffers(ID3D11Device*);
	void ShutdownBuffers();
	bool UpdateBuffers(ID3D11DeviceContext*, int, int);
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTextures(ID3D11Device*, ID3D11DeviceContext*, const std::vector<std::string>&);
	void ReleaseTextures();

	void UpdateAnimation(float frameTime);

private:
	ID3D11Buffer* m_vertexBuffer;
	ID3D11Buffer* m_indexBuffer;
	int m_indexCount;
	int m_vertexCount;

	std::vector<Texture*> m_textures;
	int m_currentFrame;
	float m_cycleTimeMs;
	float m_elapsedTimeMs;

	int m_screenWidth, m_screenHeight;
	int m_bitmapWidth, m_bitmapHeight;

	int m_previousPosX, m_previousPosY;
};