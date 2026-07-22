#include "Graphics.hpp"

Graphics::Graphics()
{
	m_direct3D = 0;
}

Graphics::~Graphics()
{

}

bool Graphics::Initialize(int width, int height, HWND hwnd)
{
	bool result;

	// create Direct3D
	m_direct3D = new D3DContext{};
	if (!m_direct3D)
	{
		return false;
	}

	result = m_direct3D->Initialize(width, height, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, "Could not initialize Direct3D.", "Error", MB_OK);
		return false;
	}

	return true;
}

void Graphics::Shutdown()
{
	if (m_direct3D)
	{
		m_direct3D->Shutdown();
		delete m_direct3D;
		m_direct3D = 0;
	}
}

bool Graphics::Frame()
{
	return Render();
}

bool Graphics::Render()
{
	// Clear
	m_direct3D->BeginScene(0.5f, 1.0f, 0.5f, 1.0f);

	// Nothing to draw yet

	m_direct3D->EndScene();

	return true;
}