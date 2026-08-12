#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

/**
* This class owns every global Direct3D object such as Device, Context and Swapchain.
* From here every other class (Shaders, Buffers, Textures) will ask Graphics class for device and context.
*/
class D3DContext
{
public:
	D3DContext();
	~D3DContext();

	D3DContext(const D3DContext&) = delete;
	D3DContext(D3DContext&&) = delete;

	bool Initialize(int, int, bool, HWND, bool, float, float);
	void Shutdown();

	void BeginScene(float, float, float, float);
	void EndScene();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	void GetProjectionMatrix(DirectX::XMMATRIX&);
	void GetWorldMatrix(DirectX::XMMATRIX&);
	void GetOrthoMatrix(DirectX::XMMATRIX&);

	void GetVideoCardInfo(char*, int&);

	void TurnZBufferOn();
	void TurnZBufferOff();

	void TurnOnAlphaBlending();
	void TurnOffAlphaBlending();

private:
	bool m_vSyncEnabled;
	int m_videoCardMemory;
	char m_videoCardDesc[128];

	IDXGISwapChain* m_swapchain;
	ID3D11Device* m_device;
	ID3D11DeviceContext* m_deviceContext;
	ID3D11RenderTargetView* m_renderTargetView;

	ID3D11Texture2D* m_depthStencilBuffer;
	ID3D11DepthStencilState* m_depthStencilState;
	ID3D11DepthStencilState* m_depthDisabledStencilState;
	ID3D11DepthStencilView* m_depthStencilView;

	ID3D11RasterizerState* m_rasterState;

	DirectX::XMMATRIX m_projectionMatrix;
	DirectX::XMMATRIX m_worldMatrix;
	DirectX::XMMATRIX m_orthoMatrix;

	ID3D11BlendState* m_alphaEnableBlendingState;
	ID3D11BlendState* m_alphaDisableBlendingState;
};