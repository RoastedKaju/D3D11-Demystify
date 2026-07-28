#include "Graphics.hpp"
#include "Utils.hpp"

Graphics::Graphics()
{
	m_direct3D = 0;
	m_camera = 0;
	m_model = 0;
	m_colorShader = 0;
	m_textureShader = 0;
	m_lightShader = nullptr;
	m_light = nullptr;
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

	m_camera = new Camera{};
	if (!m_camera)
	{
		return false;
	}
	m_camera->SetPosition(0.0f, 0.0f, -5.0f);

	m_model = new Model{};
	if (!m_model)
	{
		return false;
	}
	result = m_model->Initialize(m_direct3D->GetDevice(), m_direct3D->GetDeviceContext(), "Models/Skull/Skull.obj", "Models/Skull/Diffuse.jpg");
	if (!result)
	{
		MessageBox(hwnd, "Could not initialize the model object.", "Error", MB_OK);
		return false;
	}

	m_colorShader = new ColorShader{};
	if (!m_colorShader)
	{
		return false;
	}
	result = m_colorShader->Initialize(m_direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, "Could not initialize the color shader object.", "Error", MB_OK);
		return false;
	}

	m_textureShader = new TextureShader{};
	if (!m_textureShader)
	{
		return false;
	}
	result = m_textureShader->Initialize(m_direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, "Could not initialize the texture shader object.", "Error", MB_OK);
		return false;
	}

	m_lightShader = new LightShader{};
	if (!m_lightShader)
	{
		return false;
	}
	result = m_lightShader->Initialize(m_direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, "Could not initialize the light shader object.", "Error", MB_OK);
		return false;
	}
	// Pure data class, which contains the light direction and position
	m_light = new Light{};
	if (!m_light)
	{
		return false;
	}
	m_light->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
	m_light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_light->SetDirection(3.0f, 0.0f, 0.3f);
	m_light->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_light->SetSpecularPower(32.0f);

	return true;
}

void Graphics::Shutdown()
{
	if (m_light)
	{
		delete m_light;
		m_light = 0;
	}

	if (m_lightShader)
	{
		m_lightShader->Shutdown();
		delete m_lightShader;
		m_lightShader = 0;
	}

	if (m_textureShader)
	{
		m_textureShader->Shutdown();
		delete m_textureShader;
		m_textureShader = 0;
	}

	if (m_colorShader)
	{
		m_colorShader->Shutdown();
		delete m_colorShader;
		m_colorShader = 0;
	}

	if (m_model)
	{
		m_model->Shutdown();
		delete m_model;
		m_model = 0;
	}

	if (m_camera)
	{
		delete m_camera;
		m_camera = 0;
	}

	if (m_direct3D)
	{
		m_direct3D->Shutdown();
		delete m_direct3D;
		m_direct3D = 0;
	}
}

bool Graphics::Frame()
{
	// Fixed increments in rotation of model
	m_rotationY += 0.03f;
	if (m_rotationY > XM_2PI)
	{
		m_rotationY -= XM_2PI;
	}

	return Render();
}

bool Graphics::Render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	bool result;

	m_direct3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);
	{
		m_camera->Render();

		//m_direct3D->GetWorldMatrix(worldMatrix);
		XMMATRIX scaleMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);
		XMMATRIX rotationMatrix = XMMatrixRotationY(m_rotationY);
		XMMATRIX translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 0.0f);

		worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

		m_camera->GetViewMatrix(viewMatrix);
		m_direct3D->GetProjectionMatrix(projectionMatrix);

		// Put the model's vertex/index buffers on pipeline
		m_model->Render(m_direct3D->GetDeviceContext());

		const DirectX::XMFLOAT4 lightDirection = XMFLOAT4(m_light->GetDirection().x, m_light->GetDirection().y, m_light->GetDirection().z, 0.0f);
		result = m_lightShader->Render(m_direct3D->GetDeviceContext(),
			m_model->GetIndexCount(),
			worldMatrix, viewMatrix, projectionMatrix,
			m_model->GetTexture(),
			m_light->GetDiffuseColor(), m_light->GetAmbientColor(), lightDirection, m_light->GetSpecularColor(), m_light->GetSpecularPower(),
			m_camera->GetPosition());

		if (!result)
		{
			return false;
		}
	}
	m_direct3D->EndScene();

	return true;
}