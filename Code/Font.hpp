#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

using namespace DirectX;

class Font
{
public:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
	};

	Font();
	~Font();

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, const std::string& fontFilename, float fontPixelHeight);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();
	float GetHeight();

	void BuildVertexArray(void* vertices, const char* sentence, float drawX, float drawY);

private:
	static const int FIRST_CHAR = 32;
	static const int NUM_CHAR = 95;

	struct CharacterType
	{
		float u0, v0, u1, v1;
		float quadLeft, quadTop, quadRight, quadBottom;
		float advance;
	};

	bool LoadFontFile(const std::string& filename, std::vector<unsigned char>& outFileData);
	bool BakeAndUpload(ID3D11Device*, ID3D11DeviceContext*, const std::vector<unsigned char>& fileData, float fontPixelHeight);
	bool CreateAtlasTexture(ID3D11Device*, ID3D11DeviceContext*, const std::vector<unsigned char>& alphaBitmap, int width, int height);

private:
	CharacterType m_font[NUM_CHAR];
	ID3D11Texture2D* m_texture;
	ID3D11ShaderResourceView* m_textureView;
	float m_fontHeight;
	float m_ascent;
};