#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdio>

#ifdef _DEBUG
#define PRINT(fmt, ...) do { \
    char buffer[512]; \
    snprintf(buffer, sizeof(buffer), fmt, __VA_ARGS__); \
    OutputDebugStringA(buffer); \
    printf("%s", buffer); \
} while(0)
#else
#define PRINT(fmt, ...) ((void)0)
#endif

namespace Utils
{
	static void InitializeConsole()
	{
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
	}

	static wchar_t* ConvertToWChar(const char* cstr)
	{
		wchar_t* wString = new wchar_t[4096];
		MultiByteToWideChar(CP_ACP, 0, cstr, -1, wString, 4096);
		return wString;
	}

}