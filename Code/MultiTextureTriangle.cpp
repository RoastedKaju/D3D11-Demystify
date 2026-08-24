#include "MultiTextureTriangle.hpp"
#include "Utils.hpp"

MultiTextureTriangle::MultiTextureTriangle()
{
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;

	m_textureA = nullptr;
	m_textureB = nullptr;

	m_vertexCount = 0;
	m_indexCount = 0;
}

bool MultiTextureTriangle::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* textureA, const char* textureB)
{
	bool result;

	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}

	result = LoadTextures(device, deviceContext, textureA, textureB);
	if (!result)
	{
		ShutdownBuffers();
		return false;
	}

	return true;
}

void MultiTextureTriangle::Shutdown()
{
	ReleaseTextures();
	ShutdownBuffers();
}

void MultiTextureTriangle::Render(ID3D11DeviceContext* deviceContext)
{
	RenderBuffers(deviceContext);
}

int MultiTextureTriangle::GetIndexCount()
{
	return m_indexCount;
}

ID3D11ShaderResourceView* MultiTextureTriangle::GetTextureA()
{
	if (!m_textureA)
	{
		return nullptr;
	}

	return m_textureA->GetTexture();
}

ID3D11ShaderResourceView* MultiTextureTriangle::GetTextureB()
{
	if (!m_textureB)
	{
		return nullptr;
	}

	return m_textureB->GetTexture();
}

bool MultiTextureTriangle::InitializeBuffers(ID3D11Device* device)
{
	VertexType vertices[3];
	unsigned long indices[3];

	D3D11_BUFFER_DESC vertexBufferDesc{};
	D3D11_BUFFER_DESC indexBufferDesc{};

	D3D11_SUBRESOURCE_DATA vertexData{};
	D3D11_SUBRESOURCE_DATA indexData{};

	HRESULT hResult;

	m_vertexCount = 3;
	m_indexCount = 3;

	// Top vertex.
	vertices[0].position = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

	vertices[0].texture = DirectX::XMFLOAT2(0.5f, 0.0f);

	vertices[0].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);

	// Bottom-left vertex.
	vertices[1].position = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f);

	vertices[1].texture = DirectX::XMFLOAT2(0.0f, 1.0f);

	vertices[1].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);

	// Bottom-right vertex.
	vertices[2].position = DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f);

	vertices[2].texture = DirectX::XMFLOAT2(1.0f, 1.0f);

	vertices[2].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;

	// Vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;

	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	hResult = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);

	if (FAILED(hResult))
	{
		return false;
	}

	// Index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;

	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	hResult = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);

	if (FAILED(hResult))
	{
		return false;
	}

	return true;
}

void MultiTextureTriangle::ShutdownBuffers()
{
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = nullptr;
	}

	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = nullptr;
	}
}

void MultiTextureTriangle::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool MultiTextureTriangle::LoadTextures(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* textureA, const char* textureB)
{
	m_textureA = new Texture{};
	m_textureB = new Texture{};

	if (!m_textureA || !m_textureB)
	{
		ReleaseTextures();
		return false;
	}

	bool result = m_textureA->Initialize(device, deviceContext, textureA);

	if (!result)
	{
		ReleaseTextures();
		return false;
	}

	result = m_textureB->Initialize(device, deviceContext, textureB);

	if (!result)
	{
		ReleaseTextures();
		return false;
	}

	PRINT("Loaded Resource: %s\n", textureA);
	PRINT("Loaded Resource: %s\n", textureB);

	return true;
}

void MultiTextureTriangle::ReleaseTextures()
{
	if (m_textureA)
	{
		m_textureA->Shutdown();

		delete m_textureA;
		m_textureA = nullptr;
	}

	if (m_textureB)
	{
		m_textureB->Shutdown();

		delete m_textureB;
		m_textureB = nullptr;
	}
}