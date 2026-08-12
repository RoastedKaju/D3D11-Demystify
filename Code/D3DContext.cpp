#include "D3DContext.hpp"

D3DContext::D3DContext()
{
	m_swapchain = 0;
	m_device = 0;
	m_deviceContext = 0;
	m_renderTargetView = 0;
	m_depthStencilBuffer = 0;
	m_depthStencilState = 0;
	m_depthStencilView = 0;
	m_rasterState = 0;
	m_alphaEnableBlendingState = 0;
	m_alphaDisableBlendingState = 0;
}

D3DContext::~D3DContext()
{

}

bool D3DContext::Initialize(int width, int height, bool vsync, HWND hwnd, bool fullscreen, float screenDepth, float screenNear)
{
	HRESULT result;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i, numerator, denominator;
	size_t stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	errno_t error;
	DXGI_SWAP_CHAIN_DESC swapchainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBufferPtr;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_VIEWPORT viewport;
	D3D11_BLEND_DESC blendStateDescription;
	float fieldOfView, screenAspect;

	m_vSyncEnabled = vsync;

	// Query the GPU for a display mode that matches our resolution
	// so, we can grab its refresh rate for our vsync
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result)) { return false; }

	// Primary Graphics Card
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result)) { return false; }

	// Primary moniter attached to that card
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result)) { return false; }

	// Ask how many display modes exist for a standard 32-bit RGBA format
	// fetch a full list into a buffer we allocated
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result)) { return false; }

	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList) { return false; }

	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result)) { return false; }

	// Iterate the list looking for mode matching our window resolution
	// remember the refresh rate as numerator/denomiator fraction
	// (e.g. 60000/1001 for 59.94Hz) - that's how DXGI expresses refresh rates.
	numerator = 0;
	denominator = 1;
	for (i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)width && displayModeList[i].Height == (unsigned int)height)
		{
			numerator = displayModeList[i].RefreshRate.Numerator;
			denominator = displayModeList[i].RefreshRate.Denominator;
		}
	}

	// Grab adapter description
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result)) { return false; }

	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	error = wcstombs_s(&stringLength, m_videoCardDesc, 128, adapterDesc.Description, 128);
	if (error != 0) { return false; }

	// Clean up
	delete[] displayModeList;
	displayModeList = 0;

	adapterOutput->Release();
	adapterOutput = 0;
	adapter->Release();
	adapter = 0;
	factory->Release();
	factory = 0;

	// Create Swapchain, Device, Context
	ZeroMemory(&swapchainDesc, sizeof(swapchainDesc));

	swapchainDesc.BufferCount = 1; // 1 back buffer (double buffering)
	swapchainDesc.BufferDesc.Width = width;
	swapchainDesc.BufferDesc.Height = height;
	swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	if (m_vSyncEnabled)
	{
		swapchainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapchainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapchainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapchainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.OutputWindow = hwnd;
	swapchainDesc.SampleDesc.Count = 1; // no MSAA
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.Windowed = !fullscreen;
	swapchainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // let driver pick the most efficient present
	swapchainDesc.Flags = 0;

	featureLevel = D3D_FEATURE_LEVEL_11_0;

	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1, D3D11_SDK_VERSION, &swapchainDesc, &m_swapchain, &m_device, NULL, &m_deviceContext);
	if (FAILED(result))
	{
		MessageBox(hwnd, "Failed to create Swapchain.", "Error", MB_OK);
		return false;
	}

	// Get back buffer and wrap it in a render target view
	// GPU cannot directly render into a texture, it renders into a view.
	// This is what RTVs/DSVs/SRVs are for
	result = m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr);
	if (FAILED(result)) { return false; }

	result = m_device->CreateRenderTargetView(backBufferPtr, NULL, &m_renderTargetView);
	if (FAILED(result)) { return false; }

	backBufferPtr->Release();
	backBufferPtr = 0;

	// Create depth/stencil buffer, this is a separate texture
	// depth testing needs its own buffer
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	depthBufferDesc.Width = width;
	depthBufferDesc.Height = height;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 24 bits depth, 8 bit stencil
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	result = m_device->CreateTexture2D(&depthBufferDesc, NULL, &m_depthStencilBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Depth stencil state are the rules how dpeth/stencil testing behaves
	// this is like a configuration object
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS; // pixel passes if it's closer than what's there

	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil ops for front facing polygons
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil ops for back-facing polygons.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if (FAILED(result)) { return false; }

	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

	// A second depth stencil state but with depth set to false
	depthStencilDesc.DepthEnable = false;
	result = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthDisabledStencilState);
	if (FAILED(result))
	{
		return false;
	}

	// Depth stencil view, like render target view, this is where GPU writes
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_device->CreateDepthStencilView(m_depthStencilBuffer, &depthStencilViewDesc, &m_depthStencilView);
	if (FAILED(result))
	{
		return false;
	}

	// Bind both views to output merger stage.
	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);

	// Rasterizer
	// This controls culling, front faces, fill modes.
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;        // don't draw the back faces of triangles
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;       // vs. D3D11_FILL_WIREFRAME
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	result = m_device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	if (FAILED(result)) { return false; }

	m_deviceContext->RSSetState(m_rasterState);

	// Blend states
	ZeroMemory(&blendStateDescription, sizeof(D3D11_BLEND_DESC));

	blendStateDescription.RenderTarget[0].BlendEnable = TRUE;
	blendStateDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	result = m_device->CreateBlendState(&blendStateDescription, &m_alphaEnableBlendingState);
	if (FAILED(result))
	{
		return false;
	}

	blendStateDescription.RenderTarget[0].BlendEnable = FALSE;

	result = m_device->CreateBlendState(&blendStateDescription, &m_alphaDisableBlendingState);
	if (FAILED(result))
	{
		return false;
	}

	// Viewport - maps clip-space
	// Without this nothing would know how to fill the window
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	m_deviceContext->RSSetViewports(1, &viewport);

	// Base matrices every shader will need later
	fieldOfView = DirectX::XM_PI / 4.0f;
	screenAspect = (float)width / (float)height;

	// Perspective projection - for regular 3D geometry.
	m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);

	// World matrix starts as identity - objects get their own world matrix later.
	m_worldMatrix = DirectX::XMMatrixIdentity();

	// Orthographic projection - used later for 2D/UI rendering (no perspective).
	m_orthoMatrix = DirectX::XMMatrixOrthographicLH((float)width, (float)height, screenNear, screenDepth);

	return true;
}

void D3DContext::Shutdown()
{
	// Direct3D throws an exception if you release a swap chain while it's
	// still in exclusive fullscreen mode, so force windowed mode first.
	if (m_swapchain)
	{
		m_swapchain->SetFullscreenState(false, NULL);
	}

	if (m_rasterState) { m_rasterState->Release(); m_rasterState = 0; }
	if (m_alphaEnableBlendingState) { m_alphaEnableBlendingState->Release(); m_alphaEnableBlendingState = 0; }
	if (m_alphaDisableBlendingState) { m_alphaDisableBlendingState->Release(); m_alphaDisableBlendingState = 0; }
	if (m_depthStencilView) { m_depthStencilView->Release(); m_depthStencilView = 0; }
	if (m_depthStencilState) { m_depthStencilState->Release(); m_depthStencilState = 0; }
	if (m_depthStencilBuffer) { m_depthStencilBuffer->Release(); m_depthStencilBuffer = 0; }
	if (m_renderTargetView) { m_renderTargetView->Release(); m_renderTargetView = 0; }
	if (m_deviceContext) { m_deviceContext->Release(); m_deviceContext = 0; }
	if (m_device) { m_device->Release(); m_device = 0; }
	if (m_swapchain) { m_swapchain->Release(); m_swapchain = 0; }
}

void D3DContext::BeginScene(float red, float green, float blue, float alpha)
{
	float color[4] = { red, green, blue, alpha };

	// Clear both buffers before drawing this frame - without this you'd see
	// leftover pixels/depth values from the previous frame ("ghosting").
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void D3DContext::EndScene()
{
	// Present the back buffer to the screen.
	// Present(1, 0) = wait for vsync; Present(0, 0) = present immediately.
	if (m_vSyncEnabled)
	{
		m_swapchain->Present(1, 0);
	}
	else
	{
		m_swapchain->Present(0, 0);
	}
}

ID3D11Device* D3DContext::GetDevice()
{
	return m_device;
}

ID3D11DeviceContext* D3DContext::GetDeviceContext()
{
	return m_deviceContext;
}

void D3DContext::GetProjectionMatrix(DirectX::XMMATRIX& projectionMatrix)
{
	projectionMatrix = m_projectionMatrix;
}

void D3DContext::GetWorldMatrix(DirectX::XMMATRIX& worldMatrix)
{
	worldMatrix = m_worldMatrix;
}

void D3DContext::GetOrthoMatrix(DirectX::XMMATRIX& orthoMatrix)
{
	orthoMatrix = m_orthoMatrix;
}

void D3DContext::GetVideoCardInfo(char* cardName, int& memory)
{
	strcpy_s(cardName, 128, m_videoCardDesc);
	memory = m_videoCardMemory;
}

void D3DContext::TurnZBufferOn()
{
	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 1);
}

void D3DContext::TurnZBufferOff()
{
	m_deviceContext->OMSetDepthStencilState(m_depthDisabledStencilState, 1);
}

void D3DContext::TurnOnAlphaBlending()
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_deviceContext->OMSetBlendState(m_alphaEnableBlendingState, blendFactor, 0xffffffff);
}

void D3DContext::TurnOffAlphaBlending()
{
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_deviceContext->OMSetBlendState(m_alphaDisableBlendingState, blendFactor, 0xffffffff);
}