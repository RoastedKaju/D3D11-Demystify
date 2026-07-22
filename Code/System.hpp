#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "Input.hpp"
#include "Graphics.hpp"

class System
{
public:
	System();
	~System();

	System(const System&) = delete;
	System(System&&) = delete;

	bool Initialize();
	void Shutdown();
	void Run();

	// Handles windows messages that come through the WndProc for this application
	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();

private:
	LPCSTR m_applicationName;
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	Input* m_input;
	Graphics* m_graphics;
};

// Forward-declared free functions: raw Win32 callback the OS calls directly.
// It forwards everything into our System::MessageHandler
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global pointer to System instance so the static WndProc can reach it.
static System* ApplicationHandle = 0;