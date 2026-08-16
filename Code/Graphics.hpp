#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <DirectXMath.h>

#include "D3DContext.hpp"
#include "Camera.hpp"
#include "Model.hpp"
#include "ColorShader.hpp"
#include "TextureShader.hpp"
#include "LightShader.hpp"
#include "Light.hpp"
#include "MultiLightShader.hpp"
#include "Bitmap.hpp"
#include "Sprite.hpp"
#include "Font.hpp"
#include "FontShader.hpp"
#include "Text.hpp"
#include "FPS.hpp"

// Global rendering constants
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class Graphics
{
public:
	Graphics();
	~Graphics();

	Graphics(const Graphics&) = delete;
	Graphics(Graphics&&) = delete;

	bool Initialize(int, int, HWND);
	void Shutdown();
	bool Frame(float);

private:
	bool Render(float);

private:
	D3DContext* m_direct3D;
	Camera* m_camera;
	Model* m_model;
	ColorShader* m_colorShader;
	LightShader* m_lightShader;
	Light* m_light;

	MultiLightShader* m_multiLightShader;
	Light* m_lights[NUM_LIGHTS]; // 4 lights

	TextureShader* m_textureShader;
	Bitmap* m_bitmap;
	Sprite* m_sprite;

	Font* m_font;
	FontShader* m_fontShader;
	Text* m_text;
	int m_rotationTextIndex;

	FPS* m_FPS;
	int m_FPSTextIndex;

	float m_rotationY;
};