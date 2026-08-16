#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <mmsystem.h>

class FPS
{
public:
	FPS();
	~FPS();

	void Initialize();
	void Frame();
	int GetFPS();

private:
	int m_FPS;
	int m_count;
	unsigned long m_startTime;
};