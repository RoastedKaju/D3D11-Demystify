#include "Scene.hpp"

#include <sstream>
#include <iomanip>

/// <Scene Alpha>
SceneAlpha::SceneAlpha()
{
	m_context = nullptr;
	m_camera = nullptr;
	m_model = nullptr;
	m_colorShader = nullptr;
	m_textureShader = nullptr;
	m_lightShader = nullptr;
	m_light = nullptr;
	m_bitmap = nullptr;
	m_sprite = nullptr;
	m_font = nullptr;
	m_fontShader = nullptr;
	m_text = nullptr;
	m_rotationTextIndex = -1;
	m_FPS = nullptr;
	m_FPSTextIndex = -1;
	m_multiTextureShader = nullptr;
	m_multiTextureTriangle = nullptr;
	m_alphaMapShader = nullptr;
	m_quadMesh = nullptr;

	m_multiLightShader = nullptr;
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		m_lights[i] = 0;
	}

	m_rotationY = 0.0f;
}

bool SceneAlpha::Initialize(int width, int height, HWND hwnd, D3DContext* context)
{
	bool result;

	m_context = context;

	m_camera = new Camera{};
	if (!m_camera)
	{
		return false;
	}
	m_camera->SetPosition(0.0f, 0.0f, -5.0f);
	// Capture the view matrix at starting point as the fixed base.
	// This is set once and only updated if you move camera
	m_camera->RenderBaseViewMatrix();

	m_model = new Model{};
	if (!m_model)
	{
		return false;
	}
	result = m_model->Initialize(context->GetDevice(), context->GetDeviceContext(), "Models/Skull/Skull.obj", "Models/Skull/Diffuse.jpg");
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
	result = m_colorShader->Initialize(context->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, "Could not initialize the color shader object.", "Error", MB_OK);
		return false;
	}

	m_lightShader = new LightShader{};
	if (!m_lightShader)
	{
		return false;
	}
	result = m_lightShader->Initialize(context->GetDevice(), hwnd);
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

	// Multi lights
	m_multiLightShader = new MultiLightShader{};
	if (!m_multiLightShader)
	{
		return false;
	}
	result = m_multiLightShader->Initialize(context->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, "Could not initialize the multi-light shader object.", "Error", MB_OK);
		return false;
	}

	// Four point lights at the corners of a rough box around the model
	// origin, each a different color, so it's immediately obvious on
	// screen which light is contributing where. These positions assume a
	// roughly unit-to-few-units-scale model sitting near the origin -
	// adjust them (and/or the model's scale in Render()) to taste.
	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		m_lights[i] = new Light{};
		if (!m_lights[i])
		{
			return false;
		}
	}

	m_lights[0]->SetDiffuseColor(1.0f, 0.0f, 0.0f, 1.0f);   // red
	m_lights[0]->SetPosition(-3.0f, 1.0f, -3.0f);

	m_lights[1]->SetDiffuseColor(0.0f, 1.0f, 0.0f, 1.0f);   // green
	m_lights[1]->SetPosition(3.0f, 1.0f, -3.0f);

	m_lights[2]->SetDiffuseColor(0.0f, 0.0f, 1.0f, 1.0f);   // blue
	m_lights[2]->SetPosition(-3.0f, 1.0f, 3.0f);

	m_lights[3]->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);   // white
	m_lights[3]->SetPosition(3.0f, 1.0f, 3.0f);

	m_textureShader = new TextureShader{};
	if (!m_textureShader)
	{
		return false;
	}
	result = m_textureShader->Initialize(context->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, "Could not initialize the texture shader object.", "Error", MB_OK);
		return false;
	}

	m_bitmap = new Bitmap{};
	if (!m_bitmap)
	{
		return false;
	}
	result = m_bitmap->Initialize(context->GetDevice(), context->GetDeviceContext(), width, height, "Textures/BrickWall.jpg", 128, 128);
	if (!result)
	{
		MessageBox(hwnd, "Could not initialize the bitmap object.", "Error", MB_OK);
		return false;
	}

	m_sprite = new Sprite{};
	if (!m_sprite)
	{
		return false;
	}

	{
		std::vector<std::string> spriteFrames = { "Textures/SpriteA.jpg", "Textures/SpriteB.jpg", "Textures/SpriteC.jpg", "Textures/SpriteD.jpg" };
		float cycleTimeMs = 200.0f;

		result = m_sprite->Initialize(context->GetDevice(), context->GetDeviceContext(), width, height, spriteFrames, cycleTimeMs, 128, 128);
		if (!result)
		{
			MessageBox(hwnd, "Could not initialize the sprite object.", "Error", MB_OK);
			return false;
		}
	}

	m_font = new Font{};
	if (!m_font)
	{
		return false;
	}

	result = m_font->Initialize(context->GetDevice(), context->GetDeviceContext(), "Fonts/SpaceGrotesk-Regular.ttf", 24.0f);
	if (!result)
	{
		m_font->Shutdown();
		delete m_font;
		m_font = 0;
	}
	else
	{
		m_fontShader = new FontShader{};
		if (!m_fontShader)
		{
			return false;
		}

		result = m_fontShader->Initialize(context->GetDevice(), hwnd);
		if (!result)
		{
			MessageBox(hwnd, "Could not initialize the font shader object.", "Error", MB_OK);
			return false;
		}

		m_text = new Text{};
		if (!m_text)
		{
			return false;
		}

		result = m_text->Initialize(context->GetDevice(), m_font, width, height);
		if (!result)
		{
			MessageBox(hwnd, "Could not initialize the text object.", "Error", MB_OK);
			return false;
		}

		// A static label and a live one that Render() updates every
		// frame - showing both the simple case (set once) and the useful
		// case (an FPS-counter-style readout) side by side. maxLength=32
		// reserves room for the live one to be updated with longer
		// strings later without resizing anything.
		m_FPSTextIndex = m_text->AddSentence(context->GetDevice(), context->GetDeviceContext(), "FPS: 00", 32, 20, 0, 0.0f, 1.0f, 0.0f);
		m_text->AddSentence(context->GetDevice(), context->GetDeviceContext(), "DirectX 11 Renderer", 32, 20, 20, 1.0f, 1.0f, 1.0f);
		m_rotationTextIndex = m_text->AddSentence(context->GetDevice(), context->GetDeviceContext(), "Rotation: 0.0", 32, 20, 40, 1.0f, 1.0f, 0.0f);
	}

	m_FPS = new FPS{};
	if (!m_FPS)
	{
		return false;
	}

	m_FPS->Initialize();

	// Multi-texture example
	{
		m_multiTextureShader = new MultiTextureShader{};
		if (!m_multiTextureShader)
		{
			return false;
		}
		result = m_multiTextureShader->Initialize(context->GetDevice(), hwnd, "Shaders/Lightmap.vs", "Shaders/Lightmap.ps");
		if (!result)
		{
			MessageBox(hwnd, "Could not initialize the multi-texture shader object.", "Error", MB_OK);
			return false;
		}

		m_multiTextureTriangle = new MultiTextureTriangle{};
		if (!m_multiTextureTriangle)
		{
			return false;
		}

		result = m_multiTextureTriangle->Initialize(context->GetDevice(), context->GetDeviceContext(), "Textures/BrickWall.jpg", "Textures/Lightmap.jpg");
		if (!result)
		{
			MessageBox(hwnd, "Could not initialize the multi-texture triangle object.", "Error", MB_OK);
			return false;
		}

		// Quad mesh and Alpha mapping
		m_alphaMapShader = new AlphaMapShader{};
		if (!m_alphaMapShader)
		{
			return false;
		}
		result = m_alphaMapShader->Initialize(context->GetDevice(), hwnd, "Shaders/AlphaMap.vs", "Shaders/AlphaMap.ps");
		if (!result)
		{
			MessageBox(hwnd, "Could not initialize the Alpha Map shader object.", "Error", MB_OK);
			return false;
		}


		m_quadMesh = new QuadMesh{};
		if (!m_quadMesh)
		{
			return false;
		}

		result = m_quadMesh->Initialize(context->GetDevice(), context->GetDeviceContext(), "Textures/BrickWall.jpg", "Textures/Dirt.jpg", "Textures/Alpha.jpg");
		if (!result)
		{
			MessageBox(hwnd, "Could not initialize quad mesh object.", "Error", MB_OK);
			return false;
		}
	}

	return true;
}

void SceneAlpha::Shutdown()
{
	if (m_quadMesh)
	{
		m_quadMesh->Shutdown();
		delete m_quadMesh;
		m_quadMesh = nullptr;
	}

	if (m_alphaMapShader)
	{
		m_alphaMapShader->Shutdown();
		delete m_alphaMapShader;
		m_alphaMapShader = nullptr;
	}

	if (m_multiTextureTriangle)
	{
		m_multiTextureTriangle->Shutdown();
		delete m_multiTextureTriangle;
		m_multiTextureTriangle = nullptr;
	}

	if (m_multiTextureShader)
	{
		m_multiTextureShader->Shutdown();
		delete m_multiTextureShader;
		m_multiTextureShader = nullptr;
	}

	if (m_FPS)
	{
		delete m_FPS;
		m_FPS = nullptr;
	}

	if (m_text)
	{
		m_text->Shutdown();
		delete m_text;
		m_text = nullptr;
	}

	if (m_fontShader)
	{
		m_fontShader->Shutdown();
		delete m_fontShader;
		m_fontShader = nullptr;
	}

	if (m_font)
	{
		m_font->Shutdown();
		delete m_font;
		m_font = 0;
	}

	if (m_sprite)
	{
		m_sprite->Shutdown();
		delete m_sprite;
		m_sprite = nullptr;
	}

	if (m_bitmap)
	{
		m_bitmap->Shutdown();
		delete m_bitmap;
		m_bitmap = nullptr;
	}

	if (m_textureShader)
	{
		m_textureShader->Shutdown();
		delete m_textureShader;
		m_textureShader = 0;
	}

	for (int i = 0; i < NUM_LIGHTS; i++)
	{
		if (m_lights[i])
		{
			delete m_lights[i];
			m_lights[i] = 0;
		}
	}

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
}

void SceneAlpha::Frame(float frameTime)
{
	// Fixed increments in rotation of model
	m_rotationY += 3.0f * (frameTime / 1000);
	if (m_rotationY > XM_2PI)
	{
		m_rotationY -= XM_2PI;
	}

	m_FPS->Frame();
}

bool SceneAlpha::Render(float frameTime)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix, baseViewMatrix;
	bool result;
	XMFLOAT3 lightPositions[NUM_LIGHTS];
	XMFLOAT4 lightDiffuseColors[NUM_LIGHTS];

	m_context->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	m_camera->Render();

	//m_direct3D->GetWorldMatrix(worldMatrix);
	XMMATRIX scaleMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	XMMATRIX rotationMatrix = XMMatrixRotationY(m_rotationY);
	XMMATRIX translationMatrix = XMMatrixTranslation(0.0f, 0.0f, 0.0f);

	worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	m_camera->GetViewMatrix(viewMatrix);
	m_context->GetProjectionMatrix(projectionMatrix);
	m_context->GetOrthoMatrix(orthoMatrix);
	m_camera->GetBaseViewMatrix(baseViewMatrix);

	// 1st Pass Scene

	// Put the model's vertex/index buffers on pipeline
	m_model->Render(m_context->GetDeviceContext());

	//const DirectX::XMFLOAT4 lightDirection = XMFLOAT4(m_light->GetDirection().x, m_light->GetDirection().y, m_light->GetDirection().z, 0.0f);
	//result = m_lightShader->Render(m_direct3D->GetDeviceContext(),
	//	m_model->GetIndexCount(),
	//	worldMatrix, viewMatrix, projectionMatrix,
	//	m_model->GetTexture(),
	//	m_light->GetDiffuseColor(), m_light->GetAmbientColor(), lightDirection, m_light->GetSpecularColor(), m_light->GetSpecularPower(),
	//	m_camera->GetPosition());

	for (int i = 0; i < NUM_LIGHTS; ++i)
	{
		lightPositions[i] = m_lights[i]->GetPosition();
		lightDiffuseColors[i] = m_lights[i]->GetDiffuseColor();
	}

	result = m_multiLightShader->Render(m_context->GetDeviceContext(), m_model->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_model->GetTexture(), lightPositions, lightDiffuseColors);
	if (!result)
	{
		return false;
	}

	// Triangle rendering
	scaleMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	rotationMatrix = XMMatrixRotationY(XM_PI);
	translationMatrix = XMMatrixTranslation(-3.0f, -2.0f, 3.0f);
	worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	m_multiTextureTriangle->Render(m_context->GetDeviceContext());
	result = m_multiTextureShader->Render(m_context->GetDeviceContext(), m_multiTextureTriangle->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_multiTextureTriangle->GetTextureA(), m_multiTextureTriangle->GetTextureB());
	if (!result)
	{
		return false;
	}

	// Quad mesh rendering
	scaleMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	rotationMatrix = XMMatrixRotationY(XM_PI);
	translationMatrix = XMMatrixTranslation(3.0f, -2.0f, 3.0f);
	worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	m_quadMesh->Render(m_context->GetDeviceContext());
	result = m_alphaMapShader->Render(m_context->GetDeviceContext(), m_quadMesh->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix, m_quadMesh->GetTextureA(), m_quadMesh->GetTextureB(), m_quadMesh->GetTextureAlpha());
	if (!result)
	{
		return false;
	}

	// 2nd Pass UI
	// Depth testing has to be off
	m_context->TurnZBufferOff();

	result = m_bitmap->Render(m_context->GetDeviceContext(), 150, 50);
	if (!result)
	{
		return false;
	}

	result = m_textureShader->Render(m_context->GetDeviceContext(), m_bitmap->GetIndexCount(), XMMatrixIdentity(), baseViewMatrix, orthoMatrix, m_bitmap->GetTexture());
	if (!result)
	{
		return false;
	}

	// Show sprite
	result = m_sprite->Render(m_context->GetDeviceContext(), 600, 50, frameTime);
	if (!result)
	{
		return false;
	}

	result = m_textureShader->Render(m_context->GetDeviceContext(), m_sprite->GetIndexCount(), XMMatrixIdentity(), baseViewMatrix, orthoMatrix, m_sprite->GetTexture());
	if (!result)
	{
		return false;
	}

	// Text rendering
	if (m_text)
	{
		if (m_rotationTextIndex >= 0)
		{
			std::ostringstream oss;
			oss << "Rotation: " << std::fixed << std::setprecision(2) << m_rotationY;
			m_text->UpdateSentence(m_context->GetDeviceContext(), m_rotationTextIndex, oss.str());
		}

		// Update FPS value
		if (m_FPSTextIndex >= 0)
		{
			std::ostringstream oss;
			oss << "FPS: " << m_FPS->GetFPS();
			m_text->UpdateSentence(m_context->GetDeviceContext(), m_FPSTextIndex, oss.str());
		}

		m_context->TurnOnAlphaBlending();

		result = m_text->Render(m_context->GetDeviceContext(), m_fontShader, XMMatrixIdentity(), baseViewMatrix, orthoMatrix);
		if (!result)
		{
			return false;
		}

		m_context->TurnOffAlphaBlending();
	}

	// Turn back the depth test
	m_context->TurnZBufferOn();

	m_context->EndScene();
}
/// </Scene Alpha>

/// <Scene Beta>
SceneBeta::SceneBeta()
{
	m_context = nullptr;

	m_camera = nullptr;
	m_font = nullptr;
	m_fontShader = nullptr;
	m_text = nullptr;
	m_textIndex = -1;
	m_FPS = nullptr;
	m_FPSTextIndex = -1;
}

bool SceneBeta::Initialize(int width, int height, HWND hwnd, D3DContext* context)
{
	bool result;

	m_context = context;

	m_camera = new Camera{};
	if (!m_camera)
	{
		return false;
	}
	m_camera->SetPosition(0.0f, 0.0f, -5.0f);
	m_camera->RenderBaseViewMatrix();

	m_font = new Font{};
	if (!m_font)
	{
		return false;
	}

	result = m_font->Initialize(context->GetDevice(), context->GetDeviceContext(), "Fonts/SpaceGrotesk-Regular.ttf", 24.0f);
	if (!result)
	{
		m_font->Shutdown();
		delete m_font;
		m_font = 0;
	}
	else
	{
		m_fontShader = new FontShader{};
		if (!m_fontShader)
		{
			return false;
		}

		result = m_fontShader->Initialize(context->GetDevice(), hwnd);
		if (!result)
		{
			MessageBox(hwnd, "Could not initialize the font shader object.", "Error", MB_OK);
			return false;
		}

		m_text = new Text{};
		if (!m_text)
		{
			return false;
		}

		result = m_text->Initialize(context->GetDevice(), m_font, width, height);
		if (!result)
		{
			MessageBox(hwnd, "Could not initialize the text object.", "Error", MB_OK);
			return false;
		}

		m_FPSTextIndex = m_text->AddSentence(context->GetDevice(), context->GetDeviceContext(), "FPS: 00", 32, 20, 0, 0.0f, 1.0f, 0.0f);
		m_text->AddSentence(context->GetDevice(), context->GetDeviceContext(), "DirectX 11 Renderer", 32, 20, 20, 1.0f, 1.0f, 1.0f);
		m_textIndex = m_text->AddSentence(context->GetDevice(), context->GetDeviceContext(), "Scene: None", 32, 20, 40, 1.0f, 1.0f, 0.0f);
	}

	m_FPS = new FPS{};
	if (!m_FPS)
	{
		return false;
	}

	m_FPS->Initialize();

	return true;
}

void SceneBeta::Shutdown()
{

	if (m_FPS)
	{
		delete m_FPS;
		m_FPS = nullptr;
	}

	if (m_text)
	{
		m_text->Shutdown();
		delete m_text;
		m_text = nullptr;
	}

	if (m_fontShader)
	{
		m_fontShader->Shutdown();
		delete m_fontShader;
		m_fontShader = nullptr;
	}

	if (m_font)
	{
		m_font->Shutdown();
		delete m_font;
		m_font = 0;
	}

	if (m_camera)
	{
		delete m_camera;
		m_camera = 0;
	}
}

void SceneBeta::Frame(float frameTime)
{
	m_FPS->Frame();
}

bool SceneBeta::Render(float frameTime)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix, baseViewMatrix;
	bool result;

	// Begin
	m_context->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	m_camera->Render();
	m_camera->GetViewMatrix(viewMatrix);
	m_context->GetProjectionMatrix(projectionMatrix);
	m_context->GetOrthoMatrix(orthoMatrix);
	m_camera->GetBaseViewMatrix(baseViewMatrix);

	// turn off Z-Buffer for UI rendering
	m_context->TurnZBufferOff();

	// Text rendering
	if (m_text)
	{
		// Update text
		if (m_textIndex >= 0)
		{
			std::ostringstream oss;
			oss << "Scene: " << "Beta";
			m_text->UpdateSentence(m_context->GetDeviceContext(), m_textIndex, oss.str());
		}

		// Update FPS value
		if (m_FPSTextIndex >= 0)
		{
			std::ostringstream oss;
			oss << "FPS: " << m_FPS->GetFPS();
			m_text->UpdateSentence(m_context->GetDeviceContext(), m_FPSTextIndex, oss.str());
		}

		// render text by turning on alpha blending
		m_context->TurnOnAlphaBlending();
		result = m_text->Render(m_context->GetDeviceContext(), m_fontShader, XMMatrixIdentity(), baseViewMatrix, orthoMatrix);
		if (!result)
		{
			return false;
		}

		m_context->TurnOffAlphaBlending();
	}

	// Turn back the depth test
	m_context->TurnZBufferOn();
	m_context->EndScene();


	return true;
}

/// </Scene Beta>