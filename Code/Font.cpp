#include "Font.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include <TrueType.h>

#include <fstream>

Font::Font()
{
	m_texture = nullptr;
	m_textureView = nullptr;
	m_fontHeight = 0.0f;
	m_ascent = 0.0f;
}

Font::~Font()
{

}

bool Font::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::string& fontFilename, float fontPixelHeight)
{
	std::vector<unsigned char> fileData;
	bool result;

	m_fontHeight = fontPixelHeight;

	result = LoadFontFile(fontFilename, fileData);
	if (!result)
	{
		return false;
	}

	result = BakeAndUpload(device, deviceContext, fileData, fontPixelHeight);
	if (!result)
	{
		return false;
	}

	return true;
}

void Font::Shutdown()
{
	if (m_textureView)
	{
		m_textureView->Release();
		m_textureView = nullptr;
	}

	if (m_texture)
	{
		m_texture->Release();
		m_texture = nullptr;
	}
}

ID3D11ShaderResourceView* Font::GetTexture()
{
	return m_textureView;
}

float Font::GetHeight()
{
	return m_fontHeight;
}

bool Font::LoadFontFile(const std::string& filename, std::vector<unsigned char>& outFileData)
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	std::streamsize size;

	if (!file.is_open())
	{
		return false;
	}

	size = file.tellg();
	file.seekg(0, std::ios::beg);

	if (size <= 0)
	{
		return false;
	}

	outFileData.resize((size_t)size);

	if (!file.read((char*)outFileData.data(), size))
	{
		return false;
	}

	return true;
}

bool Font::BakeAndUpload(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::vector<unsigned char>& fileData, float fontPixelHeight)
{
	const int ATLAS_START_SIZE = 256;
	const int ATLAS_MAX_SIZE = 2048;

	std::vector<unsigned char> bitmap;
	stbtt_bakedchar bakedChars[NUM_CHAR];
	int bakeResult;
	int atlasSize;
	stbtt_fontinfo fontInfo;
	float scale;
	int ascentUnscaled, descentUnscaled, lineGapUnscaled;

	bakeResult = 0;
	for (atlasSize = ATLAS_START_SIZE; atlasSize <= ATLAS_MAX_SIZE; atlasSize *= 2)
	{
		bitmap.assign((size_t)atlasSize * atlasSize, 0);

		bakeResult = stbtt_BakeFontBitmap(fileData.data(), 0, fontPixelHeight, bitmap.data(),
			atlasSize, atlasSize, FIRST_CHAR, NUM_CHAR, bakedChars);

		if (bakeResult > 0)
		{
			break;
		}
	}

	if (bakeResult <= 0)
	{
		return false;
	}

	if (!stbtt_InitFont(&fontInfo, fileData.data(), stbtt_GetFontOffsetForIndex(fileData.data(), 0)))
	{
		return false;
	}

	scale = stbtt_ScaleForPixelHeight(&fontInfo, fontPixelHeight);
	stbtt_GetFontVMetrics(&fontInfo, &ascentUnscaled, &descentUnscaled, &lineGapUnscaled);
	m_ascent = (float)ascentUnscaled * scale;

	for (int i = 0; i < NUM_CHAR; i++)
	{
		const stbtt_bakedchar& bc = bakedChars[i];

		m_font[i].u0 = (float)bc.x0 / (float)atlasSize;
		m_font[i].v0 = (float)bc.y0 / (float)atlasSize;
		m_font[i].u1 = (float)bc.x1 / (float)atlasSize;
		m_font[i].v1 = (float)bc.y1 / (float)atlasSize;

		m_font[i].quadLeft = bc.xoff;
		m_font[i].quadRight = bc.xoff + (bc.x1 - bc.x0);
		m_font[i].quadTop = -bc.yoff;
		m_font[i].quadBottom = -bc.yoff - (bc.y1 - bc.y0);

		m_font[i].advance = bc.xadvance;
	}

	return CreateAtlasTexture(device, deviceContext, bitmap, atlasSize, atlasSize);
}

bool Font::CreateAtlasTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::vector<unsigned char>& alphaBitmap, int width, int height)
{
	std::vector<unsigned char> rgba((size_t)width * height * 4);
	D3D11_TEXTURE2D_DESC textureDesc;
	D3D11_SUBRESOURCE_DATA initData;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	HRESULT result;

	for (int i = 0; i < alphaBitmap.size(); i++)
	{
		rgba[i * 4 + 0] = 255;
		rgba[i * 4 + 1] = 255;
		rgba[i * 4 + 2] = 255;
		rgba[i * 4 + 3] = alphaBitmap[i];
	}

	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	initData.pSysMem = rgba.data();
	initData.SysMemPitch = width * 4;
	initData.SysMemSlicePitch = 0;

	result = device->CreateTexture2D(&textureDesc, &initData, &m_texture);
	if (FAILED(result))
	{
		return false;
	}

	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	result = device->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void Font::BuildVertexArray(void* vertices, const char* sentence, float drawX, float drawY)
{
	VertexType* vertexPtr;
	int numLetters, index, letter, i, v;
	float baseline, pen, advance, left, right, top, bottom;

	vertexPtr = (VertexType*)vertices;
	numLetters = (int)strlen(sentence);
	index = 0;

	baseline = drawY - m_ascent;
	pen = drawX;

	for (i = 0; i < numLetters; i++)
	{
		letter = ((int)sentence[i]) - 32;

		if (letter < 0 || letter >= NUM_CHAR || letter == 0)
		{
			advance = (letter == 0) ? m_font[0].advance : 0.0f;

			for (v = 0; v < 6; v++)
			{
				vertexPtr[index].position = XMFLOAT3(pen, baseline, 0.0f);
				vertexPtr[index].texture = XMFLOAT2(0.0f, 0.0f);
				index++;
			}

			pen += advance;
			continue;
		}

		const CharacterType& c = m_font[letter];

		left = pen + c.quadLeft;
		right = pen + c.quadRight;
		top = baseline + c.quadTop;
		bottom = baseline + c.quadBottom;

		vertexPtr[index].position = XMFLOAT3(left, top, 0.0f);
		vertexPtr[index].texture = XMFLOAT2(c.u0, c.v0);
		index++;

		vertexPtr[index].position = XMFLOAT3(right, bottom, 0.0f);
		vertexPtr[index].texture = XMFLOAT2(c.u1, c.v1);
		index++;

		vertexPtr[index].position = XMFLOAT3(left, bottom, 0.0f);
		vertexPtr[index].texture = XMFLOAT2(c.u0, c.v1);
		index++;

		vertexPtr[index].position = XMFLOAT3(left, top, 0.0f);
		vertexPtr[index].texture = XMFLOAT2(c.u0, c.v0);
		index++;

		vertexPtr[index].position = XMFLOAT3(right, top, 0.0f);
		vertexPtr[index].texture = XMFLOAT2(c.u1, c.v0);
		index++;

		vertexPtr[index].position = XMFLOAT3(right, bottom, 0.0f);
		vertexPtr[index].texture = XMFLOAT2(c.u1, c.v1);
		index++;

		pen += c.advance;
	}
}