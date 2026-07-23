#include "System.hpp"
#include "Utils.hpp"

System::System()
{
	m_input = nullptr;
	m_graphics = nullptr;

#ifdef _DEBUG
	Utils::InitializeConsole();
#endif
}

System::~System()
{
}

bool System::Initialize()
{
	int screenWidth = 0;
	int screenHeight = 0;
	bool result;

	// Create Win32 window, fill in screen size
	InitializeWindows(screenWidth, screenHeight);

	// Input
	m_input = new Input{};
	if (!m_input)
	{
		return false;
	}
	m_input->Initialize();

	// Graphics
	m_graphics = new Graphics{};
	if (!m_graphics)
	{
		return false;
	}

	result = m_graphics->Initialize(screenWidth, screenHeight, m_hWnd);
	if (!result)
	{
		return false;
	}

	return true;
}

void System::Shutdown()
{
	// Shutdown Graphics
	if (m_graphics)
	{
		m_graphics->Shutdown();
		delete m_graphics;
		m_graphics = nullptr;
	}

	// Shutdown Input
	if (m_input)
	{
		delete m_input;
		m_input = nullptr;
	}

	ShutdownWindows();
}

void System::Run()
{
	MSG message;
	bool quit{ false };
	bool result{ false };

	ZeroMemory(&message, sizeof(MSG));

	while (!quit)
	{
		// Peek message
		if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		if (message.message == WM_QUIT)
		{
			quit = true;
		}
		else
		{
			result = Frame();
			if (!result)
			{
				quit = true;
			}
		}
	}
}

LRESULT System::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
	case WM_KEYDOWN:
		m_input->KeyDown((unsigned int)wparam);
		return 0;
	case WM_KEYUP:
		m_input->KeyUp((unsigned int)wparam);
		return 0;
	default:
		return DefWindowProc(hwnd, umsg, wparam, lparam);

	}
}

bool System::Frame()
{
	bool result;

	// Escape quits application, handled here rather than through Windows messages.
	// Using the input class
	if (m_input->IsKeyDown(VK_ESCAPE))
	{
		return false;
	}

	result = m_graphics->Frame();
	if (!result)
	{
		return false;
	}


	return true;
}

void System::InitializeWindows(int& screenWidth, int& screenHeight)
{
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;

	ApplicationHandle = this;

	m_hInstance = GetModuleHandle(NULL);
	m_applicationName = "Engine";

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = m_applicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	RegisterClassEx(&wc);

	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (FULL_SCREEN)
	{
		// Match the desktop resolution exactly
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		posX = posY = 0;
	}
	else
	{
		// Window mode
		screenWidth = 800;
		screenHeight = 600;

		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	// No border or titlebar
	m_hWnd = CreateWindowEx(WS_EX_APPWINDOW,
		m_applicationName,
		m_applicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY,
		screenWidth, screenHeight, NULL, NULL, m_hInstance, NULL);

	ShowWindow(m_hWnd, SW_SHOW);
	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);

	ShowCursor(true);
}

void System::ShutdownWindows()
{
	ShowCursor(true);

	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	DestroyWindow(m_hWnd);
	m_hWnd = NULL;

	UnregisterClass(m_applicationName, m_hInstance);
	m_hInstance = NULL;

	ApplicationHandle = nullptr;
}

LRESULT WndProc(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	switch (umsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	default:
		return ApplicationHandle->MessageHandler(hwnd, umsg, wparam, lparam);
	}
}
