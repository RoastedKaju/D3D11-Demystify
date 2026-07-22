#pragma once

class Input
{
public:
	Input();
	Input(const Input&) = delete;
	Input(Input&&) = delete;
	~Input();

	void Initialize();

	void KeyDown(unsigned int);
	void KeyUp(unsigned int);

	bool IsKeyDown(unsigned int);

private:
	bool m_keys[256];
};