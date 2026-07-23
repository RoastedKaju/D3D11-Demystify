#include "Shader.hpp"

#include "Utils.hpp"

Shader::Shader()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_matrixBuffer = 0;
}

Shader::~Shader()
{

}

bool Shader::Initialize(ID3D11Device* device, HWND hwnd)
{
	return InitializeShader(device, hwnd, "color.vs", "color.ps");
}

void Shader::Shutdown()
{
	ShutdownShader();
}

bool Shader::Render(ID3D11DeviceContext* deviceContext, int indexCount, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
	bool result;

	// upload this frame's matrices into constant buffer
	result = SetShaderParameters(deviceContext, worldMatrix, viewMatrix, projectionMatrix);
	if (!result)
	{
		return false;
	}

	// bind the shaders, layout and issue the draw call
	RenderShader(deviceContext, indexCount);

	return true;
}

bool Shader::InitializeShader(ID3D11Device* device, HWND hwnd, const char* vsFilename, const char* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	unsigned int numElements;
	D3D11_BUFFER_DESC matrixBufferDesc;

	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	// Compile the HSLS source at runtime
	// "ColorVertexShader" is the entry point function name
	// 5_0 is the shader model 5 (Feature Level 11 hardware supports upto SM5)
	result = D3DCompileFromFile(Utils::ConvertToWChar(vsFilename), NULL, NULL, "ColorVertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		else
		{
			MessageBox(hwnd, (char*)vsFilename, "Missing Shader File", MB_OK);
		}
		return false;
	}

	result = D3DCompileFromFile(Utils::ConvertToWChar(psFilename), NULL, NULL, "ColorPixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		else
		{
			MessageBox(hwnd, (char*)vsFilename, "Missing Shader File", MB_OK);
		}
		return false;
	}

	// Turn the compiled bytecode blobs into actual shader objects
	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}

	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}

	// Describe the input layout
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;         // matches XMFLOAT3 position
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;      // matches XMFLOAT4 color
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;  // "right after the previous element"
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		return false;
	}

	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// Constant buffer
	// Dynamic usage and CPU access write because unlike model's vertex buffer this data changes every farme.
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void Shader::ShutdownShader()
{
	if (m_matrixBuffer) { m_matrixBuffer->Release(); m_matrixBuffer = 0; }
	if (m_layout) { m_layout->Release(); m_layout = 0; }
	if (m_pixelShader) { m_pixelShader->Release(); m_pixelShader = 0; }
	if (m_vertexShader) { m_vertexShader->Release(); m_vertexShader = 0; }
}

void Shader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const char* shaderFilename)
{
	char* compileErrors;
	size_t bufferSize;
	ofstream fout;

	compileErrors = (char*)(errorMessage->GetBufferPointer());
	bufferSize = errorMessage->GetBufferSize();

	// Dump HLSL compile errors into a log file next to exe.
	fout.open("shader-error.txt");
	for (size_t i = 0; i < bufferSize; i++)
	{
		fout << compileErrors[i];
	}
	fout.close();

	errorMessage->Release();
	errorMessage = 0;

	MessageBox(hwnd, "Error compiling shader. Check shader-error.txt for message.", (char*)shaderFilename, MB_OK);
}

bool Shader::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX worldMatrix, XMMATRIX viewMatrix, XMMATRIX projectionMatrix)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;

	// HLSL constant buffers store matrices column-major by default while
	// DirectXMath keeps them row-major in memory - transpose here so the
	// shader reads them correctly, rather than baking a transpose into HLSL.
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// Map = Ask the GPU for CPU write access to this buffer's memory
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	deviceContext->Unmap(m_matrixBuffer, 0);

	bufferNumber = 0;  // matches "register(b0)" / slot 0 - the only cbuffer in color.vs

	// Bind the now-updated constant buffer to the vertex shader stage.
	deviceContext->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	return true;
}

void Shader::RenderShader(ID3D11DeviceContext* deviceContext, int indexCount)
{
	// Tell the IA stage how to interpret the vertex buffer bytes.
	deviceContext->IASetInputLayout(m_layout);

	// Bind our compiled vertex/pixel shaders to their pipeline stages.
	deviceContext->VSSetShader(m_vertexShader, NULL, 0);
	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	// The actual draw call - everything above this was setup; this is what
	// finally pushes the triangle through the whole pipeline to the screen.
	deviceContext->DrawIndexed(indexCount, 0, 0);
}