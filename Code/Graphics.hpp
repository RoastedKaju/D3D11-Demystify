#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "D3DContext.hpp"

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
	bool Frame();

private:
	bool Render();

private:
	D3DContext* m_direct3D;
};