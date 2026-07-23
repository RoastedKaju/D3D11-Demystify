#pragma once

#include <DirectXMath.h>
using namespace DirectX;

/**
* Job of Camera class is to produce a view matrix from postion + rotation
* Shaders will later multiply every vertex by that matrix
*/
class Camera
{
public:
	Camera();
	~Camera();

	Camera(const Camera&) = delete;
	Camera(Camera&&) = delete;

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();

	void Render();
	void GetViewMatrix(XMMATRIX&);

private:
	float m_pX, m_pY, m_pZ;
	float m_rX, m_rY, m_rZ;
	XMMATRIX m_viewMatrix;
};