#include "QuadMesh.hpp"
#include "Utils.hpp"

QuadMesh::QuadMesh()
{
	m_vertexBuffer = nullptr;
	m_indexBuffer = nullptr;

	m_textureA = nullptr;
	m_textureB = nullptr;
	m_textureAlpha = nullptr;

	m_vertexCount = 0;
	m_indexCount = 0;
}

QuadMesh::~QuadMesh()
{

}

bool QuadMesh::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* textureA, const char* textureB, const char* textureAlpha)
{
	bool result;

	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}

	result = LoadTextures(device, deviceContext, textureA, textureB, textureAlpha);
	if (!result)
	{
		ShutdownBuffers();
		return false;
	}

	return true;
}

void QuadMesh::Shutdown()
{
	ReleaseTextures();
	ShutdownBuffers();
}

void QuadMesh::Render(ID3D11DeviceContext* deviceContext)
{
	RenderBuffers(deviceContext);
}

int QuadMesh::GetIndexCount()
{
	return m_indexCount;
}

ID3D11ShaderResourceView* QuadMesh::GetTextureA()
{
	if (!m_textureA)
	{
		return nullptr;
	}

	return m_textureA->GetTexture();
}

ID3D11ShaderResourceView* QuadMesh::GetTextureB()
{
	if (!m_textureB)
	{
		return nullptr;
	}

	return m_textureB->GetTexture();
}

ID3D11ShaderResourceView* QuadMesh::GetTextureAlpha()
{
	if (!m_textureAlpha)
	{
		return nullptr;
	}

	return m_textureAlpha->GetTexture();
}

bool QuadMesh::InitializeBuffers(ID3D11Device* device)
{
	VertexType vertices[4];
	unsigned long indices[6];

	D3D11_BUFFER_DESC vertexBufferDesc{};
	D3D11_BUFFER_DESC indexBufferDesc{};

	D3D11_SUBRESOURCE_DATA vertexData{};
	D3D11_SUBRESOURCE_DATA indexData{};

	HRESULT hResult;

	m_vertexCount = 4;
	m_indexCount = 6;

	// Top-left vertex
	vertices[0].position = DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f);
	vertices[0].texture = DirectX::XMFLOAT2(0.0f, 0.0f);
	vertices[0].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);

	// Top-right vertex
	vertices[1].position = DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f);
	vertices[1].texture = DirectX::XMFLOAT2(1.0f, 0.0f);
	vertices[1].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);

	// Bottom-left vertex
	vertices[2].position = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f);
	vertices[2].texture = DirectX::XMFLOAT2(0.0f, 1.0f);
	vertices[2].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);

	// Bottom-right vertex
	vertices[3].position = DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f);
	vertices[3].texture = DirectX::XMFLOAT2(1.0f, 1.0f);
	vertices[3].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);

	// Indices (two triangles forming a quad)
	indices[0] = 0; indices[1] = 2; indices[2] = 1; // First triangle
	indices[3] = 1; indices[4] = 2; indices[5] = 3; // Second triangle

	// Vertex buffer
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	vertexData.pSysMem = vertices;

	hResult = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(hResult)) return false;

	// Index buffer
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	indexData.pSysMem = indices;

	hResult = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(hResult)) return false;

	return true;
}

void QuadMesh::ShutdownBuffers()
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

void QuadMesh::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride = sizeof(VertexType);
	unsigned int offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool QuadMesh::LoadTextures(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* textureA, const char* textureB, const char* textureAlpha)
{
	m_textureA = new Texture{};
	m_textureB = new Texture{};
	m_textureAlpha = new Texture{};

	if (!m_textureA || !m_textureB || !m_textureAlpha)
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

	result = m_textureAlpha->Initialize(device, deviceContext, textureAlpha);

	if (!result)
	{
		ReleaseTextures();
		return false;
	}

	PRINT("Loaded Resource: %s\n", textureA);
	PRINT("Loaded Resource: %s\n", textureB);
	PRINT("Loaded Resource: %s\n", textureAlpha);

	return true;
}

void QuadMesh::ReleaseTextures()
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

	if (m_textureAlpha)
	{
		m_textureAlpha->Shutdown();

		delete m_textureAlpha;
		m_textureAlpha = nullptr;
	}
}