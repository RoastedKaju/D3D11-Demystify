#include "Bitmap.hpp"

Bitmap::Bitmap()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
	m_texture = 0;
}

Bitmap::~Bitmap()
{

}

bool Bitmap::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int screenWidth, int screenHeight, const char* textureFilename, int bitmapWitdh, int bitmapHeight)
{
	bool result;

	m_screenWidth = screenWidth;
	m_screenHeight = screenHeight;
	m_bitmapWidth = bitmapWitdh;
	m_bitmapHeight = bitmapHeight;

	m_previousPosX = -1;
	m_previousPosY = -1;

	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}

	result = LoadTexture(device, deviceContext, textureFilename);
	if (!result)
	{
		return false;
	}

	return true;
}

void Bitmap::Shutdown()
{
	ReleaseTexture();
	ShutdownBuffers();
}

bool Bitmap::Render(ID3D11DeviceContext* deviceContext, int positionX, int positionY)
{
	bool result;

	// only touch buffer if the position actually moved
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

int Bitmap::GetIndexCount()
{
	return m_indexCount;
}

ID3D11ShaderResourceView* Bitmap::GetTexture()
{
	return m_texture->GetTexture();
}

bool Bitmap::InitializeBuffers(ID3D11Device* device)
{
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA indexData;
	HRESULT result;

	m_vertexCount = 6;
	m_indexCount = 6;

	// USAGE_DYNAMIC + CPU_ACCESS_WRITE this time, not USAGE_DEFAULT like
	// every vertex buffer we've built before - this one gets rewritten by
	// the CPU whenever the bitmap's screen position changes, so it needs
	// to actually be mappable, unlike "write once at load time" buffers.
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(Vertex2DType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// No initial data (NULL) - unlike every other vertex buffer so far,
	// there's nothing meaningful to upload yet; UpdateBuffers fills it in
	// on the first Render() call instead.
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

	for (int i = 0; i < m_indexCount; i++)
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

void Bitmap::ShutdownBuffers()
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

bool Bitmap::UpdateBuffers(ID3D11DeviceContext* deviceContext, int posX, int posY)
{
	float left, right, top, bottom;
	Vertex2DType vertices[6];
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	Vertex2DType* verticesPtr;

	// we have to re-center that pixel coordinate around the screen's actual midpoint first.
	left = (float)((m_screenWidth / 2) * -1) + (float)posX;
	right = left + (float)m_bitmapWidth;
	top = (float)(m_screenHeight / 2) - (float)posY;
	bottom = top - (float)m_bitmapHeight;

	// Two triangles
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

	// Map and Upmap instead of create buffers, this buffer already exists from initialize buffers
	// we are just overwriting its contents in place.
	result = deviceContext->Map(m_vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	verticesPtr = (Vertex2DType*)mappedResource.pData;
	memcpy(verticesPtr, (void*)vertices, (sizeof(Vertex2DType) * m_vertexCount));

	deviceContext->Unmap(m_vertexBuffer, 0);

	return true;
}

void Bitmap::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(Vertex2DType);
	offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool Bitmap::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* filename)
{
	bool result;

	m_texture = new Texture{};
	if (!m_texture)
	{
		return false;
	}

	result = m_texture->Initialize(device, deviceContext, filename);
	if (!result)
	{
		return false;
	}

	return true;
}

void Bitmap::ReleaseTexture()
{
	if (m_texture)
	{
		m_texture->Shutdown();
		delete m_texture;
		m_texture = nullptr;
	}
}