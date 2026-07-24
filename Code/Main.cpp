#include "System.hpp"

#include <objbase.h>

/**
* The main entry point of our program, here WinMain function exists.
* WinMain is the equivalent of "main()" for a GUI application.
* It is called by the OS loader once the process starts.
*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdShow)
{
	System* system{nullptr};
	bool result{false};

	HRESULT hResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (FAILED(hResult))
	{
		return 1;
	}

	// Create the system Object
	system = new System{};
	if (!system)
	{
		return 1;
	}

	// Initialize system and run
	result = system->Initialize();
	if (result)
	{
		system->Run();
	}

	// Release system object once run returns
	system->Shutdown();
	delete system;
	system = nullptr;

	CoUninitialize();

	return 0;
}