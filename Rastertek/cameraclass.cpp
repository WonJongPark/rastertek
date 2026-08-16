////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "cameraclass.h"

CameraClass::CameraClass()
{
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_positionZ = 0.0f;

	m_rotationX = 0.0f;
	m_rotationY = 0.0f;
	m_rotationZ = 0.0f;
}

CameraClass::CameraClass(const CameraClass& other)
{}

CameraClass::~CameraClass()
{}

void CameraClass::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
	return;
}

void CameraClass::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
	return;
}

XMFLOAT3 CameraClass::GetPosition()
{
	return XMFLOAT3(m_positionX, m_positionY, m_positionZ);
}

XMFLOAT3 CameraClass::GetRotation()
{
	return XMFLOAT3(m_rotationX, m_rotationY, m_rotationZ);
}

void CameraClass::Render()
{
	XMFLOAT3 up, position, lookAt;
	XMVECTOR upVector, positionVector, lookAtVector;
	float yaw, pitch, roll;
	XMMATRIX rotationMatrix;


	// 위를 가리키는 벡터를 설정.
	up.x = 0.0f;
	up.y = 1.0f;
	up.z = 0.0f;

	// XMVECTOR 구조체에 적재.
	upVector = XMLoadFloat3(&up);

	// 월드 공간에서 카메라의 위치를 설정.
	position.x = m_positionX;
	position.y = m_positionY;
	position.z = m_positionZ;

	// XMVECTOR 구조체에 적재.
	positionVector = XMLoadFloat3(&position);

	// 카메라가 기본으로 바라보는 방향을 설정.
	lookAt.x = 0.0f;
	lookAt.y = 0.0f;
	lookAt.z = 1.0f;

	// XMVECTOR 구조체에 적재.
	lookAtVector = XMLoadFloat3(&lookAt);

	// yaw(Y축)·pitch(x축)·roll(Z축) 회전을 라디안으로 설정.
	pitch = m_rotationX * 0.0174532925f;
	yaw = m_rotationY * 0.0174532925f;
	roll = m_rotationZ * 0.0174532925f;

	// yaw·pitch·roll 값으로 회전 행렬을 만듬.
	rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// 시점이 원점에서 올바르게 회전하도록 lookAt과 up 벡터를 회전 행렬로 변환.
	lookAtVector = XMVector3TransformCoord(lookAtVector, rotationMatrix);
	upVector = XMVector3TransformCoord(upVector, rotationMatrix);

	// 회전된 카메라 위치를 관찰자의 위치로 이동.
	lookAtVector = XMVectorAdd(positionVector, lookAtVector);

	// 마지막으로 갱신된 세 벡터로 뷰 행렬을 만듬.
	m_viewMatrix = XMMatrixLookAtLH(positionVector, lookAtVector, upVector);

	return;
}

void CameraClass::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
	return;
}
