#include "Model.hpp"

Model::Model()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
}

Model::~Model()
{

}

bool Model::Initialize(ID3D11Device* device)
{
	return InitializeBuffers(device);
}

void Model::Shutdown()
{
	ShutdownBuffers();
}

void Model::Render(ID3D11DeviceContext* deviceContext)
{
	RenderBuffers(deviceContext);
}

int Model::GetIndexCount()
{
	return m_indexCount;
}

bool Model::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;
	D3D11_SUBRESOURCE_DATA indexData;
	HRESULT result;

	m_vertexCount = 3;
	m_indexCount = 3;

	vertices = new VertexType[m_vertexCount];
	if (!vertices)
	{
		return false;
	}

	indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

	vertices[0].position = XMFLOAT3(-1.0f, -1.0f, 0.0f);  // bottom left
	vertices[0].color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);  // red

	vertices[1].position = XMFLOAT3(0.0f, 1.0f, 0.0f);    // top middle
	vertices[1].color = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);  // green

	vertices[2].position = XMFLOAT3(1.0f, -1.0f, 0.0f);   // bottom right
	vertices[2].color = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);  // blue

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;

	// Vertex Buffer
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;	// GPU read/write, no CPU access
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// SubresourceData is how you give a buffer its *initial* contents at
	// creation time - required here since USAGE_DEFAULT can't be mapped later.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Index Buffer
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

	// Release the CPU arrays
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}

void Model::ShutdownBuffers()
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

void Model::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	stride = sizeof(VertexType);
	offset = 0;

	// Bind our vertex buffer to input slot 0 of the Input Assembler stage.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// Bind the index buffer too - R32_UINT because our indices are unsigned long (32-bit).
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Tell the IA stage to interpret every 3 indices as one triangle.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}