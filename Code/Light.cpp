#include "Light.hpp"

Light::Light()
{

}

Light::~Light()
{

}

void Light::SetAmbientColor(float red, float green, float blue, float alpha)
{
	m_ambientColor = XMFLOAT4(red, green, blue, alpha);
}

void Light::SetDiffuseColor(float red, float green, float blue, float alpha)
{
	m_diffuseColor = XMFLOAT4(red, green, blue, alpha);
}

void Light::SetDirection(float x, float y, float z)
{
	m_direction = XMFLOAT3(x, y, z);
}

void Light::SetSpecularColor(float red, float blue, float green, float alpha)
{
	m_specularColor = XMFLOAT4(red, green, blue, alpha);
}

void Light::SetSpecularPower(float power)
{
	m_specularPower = power;
}

XMFLOAT4 Light::GetAmbientColor() const
{
	return m_ambientColor;
}

XMFLOAT4 Light::GetDiffuseColor() const
{
	return m_diffuseColor;
}

XMFLOAT3 Light::GetDirection() const
{
	return m_direction;
}

XMFLOAT4 Light::GetSpecularColor() const
{
	return m_specularColor;
}

float Light::GetSpecularPower() const
{
	return m_specularPower;
}