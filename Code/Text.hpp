#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

#include "Font.hpp"
#include "FontShader.hpp"

using namespace DirectX;

// Manages any number of on-screen text strings ("sentences"), each with
// its own small dynamic vertex/index buffer reserved for a maximum
// character count - so a specific sentence's content (and only that
// sentence) can be changed later without recreating buffers, the way an
// FPS counter or debug readout needs to update every frame while
// everything else on screen stays untouched.
class Text
{
public:
	Text();
	~Text();

	bool Initialize(ID3D11Device*, Font*, int, int);
	void Shutdown();

	int AddSentence(ID3D11Device*, ID3D11DeviceContext*, const std::string&, int maxLength, int positionX, int positionY, float red, float green, float blue);

	bool UpdateSentence(ID3D11DeviceContext*, int sentenceIndex, const std::string&);
	bool Render(ID3D11DeviceContext*, FontShader*, XMMATRIX, XMMATRIX, XMMATRIX);

private:
	struct SentenceType
	{
		ID3D11Buffer* vertexBuffer;
		ID3D11Buffer* indexBuffer;
		int maxLength;
		int vertexCount, indexCount;
		int positionX, positionY;
		float red, green, blue;
	};

	bool InitializeSentenceBuffers(ID3D11Device*, SentenceType&, int maxLength);
	bool UpdateSentenceBuffers(ID3D11DeviceContext*, SentenceType&, const std::string&);
	void ShutdownSentence(SentenceType&);

private:
	Font* m_Font;   // not owned - just borrowed, GraphicsClass owns the real one
	int m_screenWidth, m_screenHeight;
	std::vector<SentenceType> m_Sentences;
};