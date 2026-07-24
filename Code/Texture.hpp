#pragma once

#include <d3d11.h>
#include <wincodec.h> // WIC - Windows image loader

/**
* Load image file, turn it into a GPU texture and shader resource view
*/
class Texture
{
public:
	Texture();
	~Texture();

	bool Initialize(ID3D11Device*, ID3D11DeviceContext*, const char*);
	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();

private:
	bool LoadImageWIC(const char*);

private:
	unsigned char* m_imageData;
	ID3D11Texture2D* m_texture;
	ID3D11ShaderResourceView* m_textureView;
	int m_width;
	int m_height;
};