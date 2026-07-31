#include "Camera.hpp"

Camera::Camera()
{
	m_pX = 0.0f;
	m_pY = 0.0f;
	m_pZ = 0.0f;

	m_rX = 0.0f;
	m_rY = 0.0f;
	m_rZ = 0.0f;
}

Camera::~Camera()
{

}

void Camera::SetPosition(float x, float y, float z)
{
	m_pX = x;
	m_pY = y;
	m_pZ = z;
}

void Camera::SetRotation(float x, float y, float z)
{
	m_rX = x;
	m_rY = y;
	m_rZ = z;
}

XMFLOAT3 Camera::GetPosition()
{
	return XMFLOAT3(m_pX, m_pY, m_pZ);
}

XMFLOAT3 Camera::GetRotation()
{
	return XMFLOAT3(m_rX, m_rY, m_rZ);
}

void Camera::Render()
{
	XMFLOAT3 up, position, lookAt;
	XMVECTOR upVec, PosVec, lookAtVec;
	float yaw, pitch, roll;
	XMMATRIX rotationMatrix;

	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;
	upVec = XMLoadFloat3(&up);

	position.x = m_pX;
	position.y = m_pY;
	position.z = m_pZ;
	PosVec = XMLoadFloat3(&position);

	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;
	lookAtVec = XMLoadFloat3(&lookAt);

	// Convert degrees to radians
	pitch = m_rX * 0.0174532925f;
	yaw = m_rY * 0.0174532925f;
	roll = m_rZ * 0.0174532925f;

	rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// Rotate default look up vectors by camera's current rotation
	// then translate lookAt into world space relative to camera position
	lookAtVec = XMVector3TransformCoord(lookAtVec, rotationMatrix);
	upVec = XMVector3TransformCoord(upVec, rotationMatrix);

	lookAtVec = XMVectorAdd(PosVec, lookAtVec);

	// build view matrix from eye position to target
	m_viewMatrix = XMMatrixLookAtLH(PosVec, lookAtVec, upVec);
}

void Camera::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
}

void Camera::RenderBaseViewMatrix()
{
	XMFLOAT3 up, position, lookAt;
	XMVECTOR upVec, posVec, lookAtVec;
	float yaw, pitch, roll;
	XMMATRIX rotationMatrix;

	// Identical to render above - the only difference is when this gets called.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;
	upVec = XMLoadFloat3(&up);

	position.x = m_pX;
	position.y = m_pY;
	position.z = m_pZ;
	posVec = XMLoadFloat3(&position);

	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;
	lookAtVec = XMLoadFloat3(&lookAt);

	// Degrees -> radians (DirectXMath trig functions all expect radians).
	pitch = m_rX * 0.0174532925f;
	yaw = m_rY * 0.0174532925f;
	roll = m_rZ * 0.0174532925f;

	rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	lookAtVec = XMVector3TransformCoord(lookAtVec, rotationMatrix);
	upVec = XMVector3TransformCoord(upVec, rotationMatrix);

	lookAtVec = XMVectorAdd(posVec, lookAtVec);

	m_baseViewMatrix = XMMatrixLookAtLH(posVec, lookAtVec, upVec);
}

void Camera::GetBaseViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_baseViewMatrix;
}