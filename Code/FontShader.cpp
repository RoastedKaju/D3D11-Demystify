#include "FontShader.hpp"
#include "Utils.hpp"

FontShader::FontShader()
{
	m_vertexShader = nullptr;
	m_pixelShader = nullptr;
	m_layout = nullptr;
	m_matrixBuffer = nullptr;
	m_sampleState = nullptr;
	m_pixelBuffer = nullptr;
}

FontShader::~FontShader()
{

}

bool FontShader::Initialize(ID3D11Device* device, HWND hwnd)
{
	return InitializeShader(device, hwnd, "Shaders/Font.vs", "Shaders/Font.ps");
}

void FontShader::Shutdown()
{
	ShutdownShader();
}

bool FontShader::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, XMFLOAT4 pixelColor)
{
	bool result;

	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix, texture, pixelColor);
	if (!result)
	{
		return false;
	}

	RenderShader(deviceContext, indexCount);

	return true;
}

bool FontShader::InitializeShader(ID3D11Device* device, HWND hwnd, const char* vsFilename, const char* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC pixelBufferDesc;

	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	result = D3DCompileFromFile(Utils::ConvertToWChar(vsFilename), NULL, NULL, "FontVertexShader", "vs_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage) { OutputShaderErrorMessage(errorMessage, hwnd, vsFilename); }
		else { MessageBox(hwnd, vsFilename, "Missing Shader File", MB_OK); }
		return false;
	}

	result = D3DCompileFromFile(Utils::ConvertToWChar(psFilename), NULL, NULL, "FontPixelShader", "ps_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
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

	// Same 2-element layout as TextureShaderClass - position + texcoord,
	// matching FontClass::VertexType exactly.
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

	// Point filtering (not linear) is the more common choice for crisp
	// bitmap font glyphs at their native size - linear filtering softens
	// glyph edges more than most people want for text. Wrap addressing is
	// irrelevant here since UVs never leave 0..1 for a single glyph cell,
	// but kept consistent with the other texture shaders.
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

	// Pixel buffer - just one float4, the tint color for this draw call.
	pixelBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	pixelBufferDesc.ByteWidth = sizeof(PixelBufferType);
	pixelBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pixelBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pixelBufferDesc.MiscFlags = 0;
	pixelBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&pixelBufferDesc, NULL, &m_pixelBuffer);
	if (FAILED(result)) { return false; }

	return true;
}

void FontShader::ShutdownShader()
{
	if (m_pixelBuffer) { m_pixelBuffer->Release(); m_pixelBuffer = 0; }
	if (m_sampleState) { m_sampleState->Release(); m_sampleState = 0; }
	if (m_matrixBuffer) { m_matrixBuffer->Release(); m_matrixBuffer = 0; }
	if (m_layout) { m_layout->Release(); m_layout = 0; }
	if (m_pixelShader) { m_pixelShader->Release(); m_pixelShader = 0; }
	if (m_vertexShader) { m_vertexShader->Release(); m_vertexShader = 0; }
}

void FontShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const char* shaderFilename)
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

bool FontShader::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, ID3D11ShaderResourceView* texture, XMFLOAT4 pixelColor)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* matrixDataPtr;
	PixelBufferType* pixelDataPtr;
	unsigned int bufferNumber;

	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	matrixDataPtr = (MatrixBufferType*)mappedResource.pData;
	matrixDataPtr->world = worldMatrix;
	matrixDataPtr->view = viewMatrix;
	matrixDataPtr->projection = projectionMatrix;

	deviceContext->Unmap(m_matrixBuffer, 0);

	bufferNumber = 0;
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	deviceContext->PSSetShaderResources(0, 1, &texture);

	result = deviceContext->Map(m_pixelBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	pixelDataPtr = (PixelBufferType*)mappedResource.pData;
	pixelDataPtr->pixelColor = pixelColor;

	deviceContext->Unmap(m_pixelBuffer, 0);

	bufferNumber = 0;
	deviceContext->PSSetConstantBuffers(bufferNumber, 1, &m_pixelBuffer);

	return true;
}

void FontShader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	deviceContext->IASetInputLayout(m_layout);

	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	deviceContext->PSSetSamplers(0, 1, &m_sampleState);

	deviceContext->DrawIndexed(indexCount, 0, 0);
}