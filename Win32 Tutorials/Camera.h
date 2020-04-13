#pragma once

#include "Graphics.h"

class Camera
{
public:
	DirectX::XMMATRIX GetMatrix() const noexcept;
	void SpawnControlWindow() noexcept;
	void Reset() noexcept;

private:
	//Distance in the Z axis
	float r = 20.0f;

	//Global rotation
	float theta = 0.0f;
	float phi = 0.0f;

	//Local Rotation
	float pitch = 0.0f;
	float yaw = 0.0f;
	float roll = 0.0f;
};

