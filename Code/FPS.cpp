#include "FPS.hpp"

FPS::FPS()
{
	m_FPS = 0;
	m_count = 0;
	m_startTime = 0;
}

FPS::~FPS()
{

}

void FPS::Initialize()
{
	m_FPS = 0;
	m_count = 0;
	m_startTime = timeGetTime();
}

void FPS::Frame()
{
	++m_count;

	if (timeGetTime() >= (m_startTime + 1000))
	{
		m_FPS = m_count;
		m_count = 0;

		m_startTime = timeGetTime();
	}
}

int FPS::GetFPS()
{
	return m_FPS;
}