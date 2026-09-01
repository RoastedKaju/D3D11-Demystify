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
#include "MultiTextureShader.hpp"
#include "MultiTextureTriangle.hpp"
#include "AlphaMapShader.hpp"
#include "QuadMesh.hpp"

class Scene
{
public:
	virtual bool Initialize(int width, int height, HWND hwnd, D3DContext* context) = 0;
	virtual void Shutdown() = 0;
	virtual void Frame(float frameTime) = 0;
	virtual bool Render(float frameTime) = 0;
};

// Scene Alpha: this covers my implementation of raster-tek from 0-19
class SceneAlpha : public Scene
{
public:
	SceneAlpha();

	bool Initialize(int width, int height, HWND hwnd, D3DContext* context) override;

	void Shutdown() override;

	void Frame(float frameTime) override;

	bool Render(float frameTime) override;
private:
	D3DContext* m_context;

	// Scene entities
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

	MultiTextureShader* m_multiTextureShader;
	MultiTextureTriangle* m_multiTextureTriangle;

	AlphaMapShader* m_alphaMapShader;
	QuadMesh* m_quadMesh;

	float m_rotationY;
};

class SceneBeta : public Scene
{
public:
	SceneBeta();

	bool Initialize(int width, int height, HWND hwnd, D3DContext* context) override;

	void Shutdown() override;

	void Frame(float frameTime) override;

	bool Render(float frameTime) override;

private:
	D3DContext* m_context;

	// Scene entities
	Camera* m_camera;
	// Text rendering
	Font* m_font;
	FontShader* m_fontShader;
	Text* m_text;
	int m_textIndex;
	FPS* m_FPS;
	int m_FPSTextIndex;
};

