#pragma once

class Light : public Object3D {
public:
	Light() = default;
	~Light() = default;

public:
	float intensity = 1.0f;
	glm::vec3 color = { 1.0f, 1.0f, 1.0f };
};

class DirectionalLight : public Light{
public:
	DirectionalLight() = default;
	~DirectionalLight() = default;
};

class SpotLight : public Light {
public:
	SpotLight() = default;

	~SpotLight() = default;

	bool isMove = false;


public:
	float angle = glm::radians(180.0f);
	float kc = 1.0f;
	float kl = 0.0f;
	float kq = 0.2f;
};