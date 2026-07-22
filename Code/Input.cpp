#include "Input.hpp"

Input::Input()
{

}

Input::~Input()
{

}

void Input::Initialize()
{
	for (auto i = 0; i < 256; ++i)
	{
		m_keys[i] = false;
	}
}

void Input::KeyDown(unsigned int input)
{
	m_keys[input] = true;
}

void Input::KeyUp(unsigned int input)
{
	m_keys[input] = false;
}

bool Input::IsKeyDown(unsigned int key)
{
	return m_keys[key];
}