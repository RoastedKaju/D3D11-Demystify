#include "MultiLightShader.hpp"
#include "Utils.hpp"

MultiLightShader::MultiLightShader()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
	m_sampleState = 0;
	m_lightPositionBuffer = 0;
	m_lightColorBuffer = 0;
}


MultiLightShader::~MultiLightShader()
{

}

bool MultiLightShader::Initialize(ID3D11Device* device, HWND hwnd)
{
	return InitializeShader(device, hwnd, "Shaders/MultiLight.vs", "Shaders/MultiLight.ps");
}

void MultiLightShader::Shutdown()
{
	ShutdownShader();
}

bool MultiLightShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX world, XMMATRIX view, XMMATRIX projection, ID3D11ShaderResourceView* texture, const XMFLOAT3* lightPositions, const XMFLOAT4* lightColors)
{
	bool result;
	result = SetShaderParameters(deviceContext, world, view, projection, texture, lightPositions, lightColors);
	if (!result)
	{
		return false;
	}

	RenderShader(deviceContext, indexCount);

	return true;
}

bool MultiLightShader::InitializeShader(ID3D11Device* device, HWND hwnd, const char* vsFilename, const char* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC lightPositionBufferDesc;
	D3D11_BUFFER_DESC lightColorBufferDesc;

	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	result = D3DCompileFromFile(Utils::ConvertToWChar(vsFilename), NULL, NULL, "MultiLightVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage) { OutputShaderErrorMessage(errorMessage, hwnd, vsFilename); }
		else { MessageBox(hwnd, vsFilename, "Missing Shader File", MB_OK); }
		return false;
	}

	result = D3DCompileFromFile(Utils::ConvertToWChar(psFilename), NULL, NULL, "MultiLightPixelShader", "ps_5_0",D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage) { OutputShaderErrorMessage(errorMessage, hwnd, psFilename); }
		else { MessageBox(hwnd, psFilename, "Missing Shader File", MB_OK); }
		return false;
	}

	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result)) { return false; }

	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(),
		pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result)) { return false; }

	// Input layout is unchanged from earlier tutorials - the lightDir[]
	// array is computed inside the vertex shader from the light position
	// buffer, it never comes from the vertex buffer itself.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(),
		vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result)) { return false; }

	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;
	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result)) { return false; }

	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result)) { return false; }

	// Light position buffer - bound to the VERTEX shader stage, since each
	// vertex needs every light's position to compute its own per-light
	// direction vectors.
	lightPositionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightPositionBufferDesc.ByteWidth = sizeof(LightPositionBufferType);
	lightPositionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightPositionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightPositionBufferDesc.MiscFlags = 0;
	lightPositionBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&lightPositionBufferDesc, NULL, &m_lightPositionBuffer);
	if (FAILED(result)) { return false; }

	// Light color buffer - bound to the PIXEL shader stage, since that's
	// where each light's contribution actually gets computed and summed.
	lightColorBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightColorBufferDesc.ByteWidth = sizeof(LightColorBufferType);
	lightColorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightColorBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightColorBufferDesc.MiscFlags = 0;
	lightColorBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&lightColorBufferDesc, NULL, &m_lightColorBuffer);
	if (FAILED(result)) { return false; }

	return true;
}

void MultiLightShader::ShutdownShader()
{
	if (m_lightColorBuffer) { m_lightColorBuffer->Release(); m_lightColorBuffer = 0; }
	if (m_lightPositionBuffer) { m_lightPositionBuffer->Release(); m_lightPositionBuffer = 0; }
	if (m_sampleState) { m_sampleState->Release(); m_sampleState = 0; }
	if (m_matrixBuffer) { m_matrixBuffer->Release(); m_matrixBuffer = 0; }
	if (m_layout) { m_layout->Release(); m_layout = 0; }
	if (m_pixelShader) { m_pixelShader->Release(); m_pixelShader = 0; }
	if (m_vertexShader) { m_vertexShader->Release(); m_vertexShader = 0; }
}

void MultiLightShader::OutputShaderErrorMessage(ID3DBlob* errorMessage, HWND hwnd, const char* shaderFilename)
{
	char* compileErrors;
	size_t bufferSize;
	std::ofstream fout;

	compileErrors = (char*)(errorMessage->GetBufferPointer());
	bufferSize = errorMessage->GetBufferSize();

	fout.open("shader-error.txt");
	for (size_t i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}
	fout.close();

	errorMessage->Release();
	errorMessage = 0;

	MessageBox(hwnd, "Error compiling shader. Check shader-error.txt for message.", shaderFilename, MB_OK);
}

bool MultiLightShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX world, XMMATRIX view, XMMATRIX projection, ID3D11ShaderResourceView* texture, const XMFLOAT3* positions, const XMFLOAT4* colors)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* matrixDataPtr;
	LightPositionBufferType* lightPositionDataPtr;
	LightColorBufferType* lightColorDataPtr;
	unsigned int bufferNumber;
	int i;

	world = XMMatrixTranspose(world);
	view = XMMatrixTranspose(view);
	projection = XMMatrixTranspose(projection);

	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	matrixDataPtr = (MatrixBufferType*)mappedResource.pData;
	matrixDataPtr->world = world;
	matrixDataPtr->view = view;
	matrixDataPtr->projection = projection;

	deviceContext->Unmap(m_matrixBuffer, 0);

	bufferNumber = 0;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	// Upload all NUM_LIGHTS positions as float4s - w is unused (just makes
	// the C++ struct match the HLSL float4 array element-for-element).
	result = deviceContext->Map(m_lightPositionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	lightPositionDataPtr = (LightPositionBufferType*)mappedResource.pData;
	for (i = 0; i < NUM_LIGHTS; i++)
	{
		lightPositionDataPtr->positions[i] = XMFLOAT4(positions[i].x, positions[i].y, positions[i].z, 1.0f);
	}

	deviceContext->Unmap(m_lightPositionBuffer, 0);

	bufferNumber = 1;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_lightPositionBuffer);

	deviceContext->PSSetShaderResources(0, 1, &texture);

	// Upload all NUM_LIGHTS diffuse colors.
	result = deviceContext->Map(m_lightColorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	lightColorDataPtr = (LightColorBufferType*)mappedResource.pData;
	for (i = 0; i < NUM_LIGHTS; i++)
	{
		lightColorDataPtr->colors[i] = colors[i];
	}

	deviceContext->Unmap(m_lightColorBuffer, 0);

	bufferNumber = 0;
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_lightColorBuffer);

	return true;
}

void MultiLightShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	deviceContext->IASetInputLayout(m_layout);

	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	deviceContext->DrawIndexed(indexCount, 0, 0);
}