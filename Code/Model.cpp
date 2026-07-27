#include "Model.hpp"
#include "Utils.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <TinyObjLoader.h>

Model::Model()
{
	m_vertexBuffer = 0;
	m_indexBuffer = 0;
}

Model::~Model()
{

}

bool Model::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* modelPath, const char* texturePath)
{
	bool result;

	result = InitializeBuffers(device, modelPath);
	if (!result)
	{
		return false;
	}

	result = LoadTexture(device, deviceContext, texturePath);
	if (!result)
	{
		return false;
	}

	return true;
}

void Model::Shutdown()
{
	ReleaseTexture();
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

ID3D11ShaderResourceView* Model::GetTexture()
{
	return m_texture->GetTexture();
}

bool Model::InitializeBuffers(ID3D11Device* device, const char* modelPath)
{
	D3D11_BUFFER_DESC vertexBufferDesc;
	D3D11_BUFFER_DESC indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData;
	D3D11_SUBRESOURCE_DATA indexData;
	HRESULT hResult;

	bool result = LoadModel(modelPath);
	if (!result)
	{
		return false;
	}

	m_vertexCount = (int)m_vertices.size();
	m_indexCount = (int)m_indices.size();

	PRINT("Vertex Count is: %d\n", m_vertexCount);

	// Vertex Buffer
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;	// GPU read/write, no CPU access
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// SubresourceData is how you give a buffer its *initial* contents at
	// creation time - required here since USAGE_DEFAULT can't be mapped later.
	vertexData.pSysMem = m_vertices.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	hResult = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(hResult))
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

	indexData.pSysMem = m_indices.data();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	hResult = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(hResult))
	{
		return false;
	}

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

bool Model::LoadTexture(ID3D11Device* device, ID3D11DeviceContext* deviceContext, const char* texturePath)
{
	bool result;

	m_texture = new Texture{};
	if (!m_texture)
	{
		return false;
	}

	result = m_texture->Initialize(device, deviceContext, texturePath);
	if (!result)
	{
		return false;
	}

	PRINT("Loaded Resource: %s\n", texturePath);
	return true;
}

bool Model::LoadModel(const char* modelPath)
{
	tinyobj::ObjReaderConfig readerConfig;
	readerConfig.triangulate = true;

	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(modelPath, readerConfig))
	{
		if (!reader.Error().empty())
		{
			PRINT("Error on loading model: %s\n", reader.Error().c_str());
		}
		return false;
	}

	const tinyobj::attrib_t& attrib = reader.GetAttrib();
	const std::vector<tinyobj::shape_t>& shapes = reader.GetShapes();

	m_vertices.clear();
	for (const auto& shape : shapes)
	{
		for (const auto& idx : shape.mesh.indices)
		{
			VertexType vertex;

			vertex.position = XMFLOAT3(
				attrib.vertices[3 * idx.vertex_index + 0],
				attrib.vertices[3 * idx.vertex_index + 1],
				attrib.vertices[3 * idx.vertex_index + 2]
			);

			if (idx.texcoord_index >= 0)
			{
				vertex.texture = XMFLOAT2(
					attrib.texcoords[2 * idx.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * idx.texcoord_index + 1]
				);
			}
			else
			{
				vertex.texture = XMFLOAT2(0.0f, 0.0f);
			}

			if (idx.normal_index >= 0)
			{
				vertex.normal = XMFLOAT3(
					attrib.normals[3 * idx.normal_index + 0],
					attrib.normals[3 * idx.normal_index + 1],
					attrib.normals[3 * idx.normal_index + 2]
				);
			}
			else
			{
				vertex.normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
			}

			m_vertices.push_back(vertex);
		}
	}

	if (m_vertices.empty())
	{
		return false;
	}

	m_indices.resize(m_vertices.size());
	for (size_t i = 0; i < m_indices.size(); i++)
	{
		m_indices[i] = (unsigned long)i;
	}

	return true;
}

void Model::ReleaseTexture()
{
	if (m_texture)
	{
		m_texture->Shutdown();
		delete m_texture;
		m_texture = 0;
	}
}