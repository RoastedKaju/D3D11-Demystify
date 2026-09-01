#include "Graphics.hpp"
#include "Utils.hpp"

Graphics::Graphics()
{
	m_direct3D = nullptr;
	m_scene = nullptr;
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

	m_scene = new SceneBeta{};
	if (!m_scene)
	{
		return false;
	}
	// create scene
	m_scene->Initialize(width, height, hwnd, m_direct3D);

	return true;
}

void Graphics::Shutdown()
{
	// tear down scene
	if (m_scene)
	{
		m_scene->Shutdown();
		delete m_scene;
		m_scene = nullptr;
	}

	if (m_direct3D)
	{
		m_direct3D->Shutdown();
		delete m_direct3D;
		m_direct3D = nullptr;
	}
}

bool Graphics::Frame(float frameTime)
{
	// update scene
	if (m_scene)
	{
		m_scene->Frame(frameTime);
	}

	return Render(frameTime);
}

bool Graphics::Render(float frameTime)
{
	// render scene
	if (m_scene)
	{
		m_scene->Render(frameTime);
	}

	return true;
}