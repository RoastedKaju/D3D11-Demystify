#include "Sprite.hpp"

Sprite::Sprite()
{
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;
	m_currentFrame = 0;
	m_cycleTimeMs = 0.0f;
	m_elapsedTimeMs = 0.0f;
}

Sprite::~Sprite()
{

}

bool Sprite::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int screenWidth, int screenHeight, const std::vector<std::string>& textureFilenames, float cycleTimeMs, int bitmapWidth, int bitmapHeight)
{
	bool result;

	m_screenHeight = screenHeight;
	m_screenWidth = screenWidth;
	m_bitmapHeight = bitmapHeight;
	m_bitmapWidth = bitmapWidth;
	m_cycleTimeMs = cycleTimeMs;

	m_previousPosX = -1;
	m_previousPosY = -1;

	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}

	result = LoadTextures(device, deviceContext, textureFilenames);
	if (!result)
	{
		return false;
	}

	return true;
}

void Sprite::Shutdown()
{
	ReleaseTextures();
	ShutdownBuffers();
}

bool Sprite::Render(ID3D11DeviceContext* deviceContext, int positionX, int positionY, float frameTime)
{
	bool result;

	UpdateAnimation(frameTime);

	if (positionX != m_previousPosX || positionY != m_previousPosY)
	{
		result = UpdateBuffers(deviceContext, positionX, positionY);
		if (!result)
		{
			return false;
		}

		m_previousPosX = positionX;
		m_previousPosY = positionY;
	}

	RenderBuffers(deviceContext);

	return true;
}

int Sprite::GetIndexCount()
{
	return m_indexCount;
}

ID3D11ShaderResourceView* Sprite::GetTexture()
{
	return m_textures[m_currentFrame]->GetTexture();
}

void Sprite::UpdateAnimation(float frameTime)
{
	// Nothing to cycle through with 0 or 1 frame - a single-texture sprite
	// just behaves like a static BitmapClass.
	if (m_textures.size() <= 1 || m_cycleTimeMs <= 0.0f)
	{
		return;
	}

	m_elapsedTimeMs += frameTime;

	while (m_elapsedTimeMs >= m_cycleTimeMs)
	{
		m_elapsedTimeMs -= m_cycleTimeMs;
		m_currentFrame = (m_currentFrame + 1) % (int)m_textures.size();
	}
}

bool Sprite::InitializeBuffers(ID3D11Device* device)
{
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA indexData;
	HRESULT result;
	int i;

	m_vertexCount = 6;
	m_indexCount = 6;

	// Same reasoning as BitmapClass: DYNAMIC + CPU_ACCESS_WRITE, no initial
	// data, since the quad's actual position gets filled in later by
	// UpdateBuffers and can change at any time.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&vertexBufferDesc, NULL, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

	for (i = 0; i < m_indexCount; i++)
	{
		indices[i] = (unsigned long)i;
	}

	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	delete[] indices;
	indices = 0;

	return true;
}

void Sprite::ShutdownBuffers()
{
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}
}

bool Sprite::UpdateBuffers(ID3D11DeviceContext* deviceContext, int positionX, int positionY)
{
	float left, right, top, bottom;
	VertexType vertices[6];
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	VertexType* verticesPtr;

	// Identical screen-pixel -> ortho-space conversion as BitmapClass.
	left = (float)((m_screenWidth / 2) * -1) + (float)positionX;
	right = left + (float)m_bitmapWidth;
	top = (float)(m_screenHeight / 2) - (float)positionY;
	bottom = top - (float)m_bitmapHeight;

	vertices[0].position = XMFLOAT3(left, top, 0.0f);
	vertices[0].texture = XMFLOAT2(0.0f, 0.0f);

	vertices[1].position = XMFLOAT3(right, bottom, 0.0f);
	vertices[1].texture = XMFLOAT2(1.0f, 1.0f);

	vertices[2].position = XMFLOAT3(left, bottom, 0.0f);
	vertices[2].texture = XMFLOAT2(0.0f, 1.0f);

	vertices[3].position = XMFLOAT3(left, top, 0.0f);
	vertices[3].texture = XMFLOAT2(0.0f, 0.0f);

	vertices[4].position = XMFLOAT3(right, top, 0.0f);
	vertices[4].texture = XMFLOAT2(1.0f, 0.0f);

	vertices[5].position = XMFLOAT3(right, bottom, 0.0f);
	vertices[5].texture = XMFLOAT2(1.0f, 1.0f);

	result = deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	verticesPtr = (VertexType*)mappedResource.pData;
	memcpy(verticesPtr, (void*)vertices, (sizeof(VertexType) * m_vertexCount));

	deviceContext->Unmap(m_vertexBuffer, 0);

	return true;
}

void Sprite::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(VertexType);
	offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool Sprite::LoadTextures(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const std::vector<std::string>& textureFilenames)
{
	bool result;

	for (const auto& filename : textureFilenames)
	{
		Texture* texture = new Texture{};
		if (!texture)
		{
			return false;
		}

		result = texture->Initialize(device, deviceContext, filename.c_str());
		if (!result)
		{
			delete texture;
			return false;
		}

		m_textures.push_back(texture);
	}

	return !m_textures.empty();
}

void Sprite::ReleaseTextures()
{
	for (auto* texture : m_textures)
	{
		if (texture)
		{
			texture->Shutdown();
			delete texture;
		}
	}

	m_textures.clear();
}