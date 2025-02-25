#pragma once

#include "object3d.h"

class Camera : public Object3D {
public:
	glm::mat4 getViewMatrix() const;
	
	virtual glm::mat4 getProjectionMatrix() const = 0;

	glm::vec3 _direction = glm::vec3(0.0f, 0.0f, 0.0f);
};


class PerspectiveCamera : public Camera {
public:
	float fovy;
	float aspect;
	float znear;
	float zfar;
public:
	PerspectiveCamera(float fovy, float aspect, float znear, float zfar);

	~PerspectiveCamera() = default;

	glm::mat4 getProjectionMatrix() const override;
};


class OrthographicCamera : public Camera {
public:
	float left;
	float right;
	float bottom;
	float top;
	float znear;
	float zfar;
public:
	OrthographicCamera(float left, float right, float bottom, float top, float znear, float zfar);
	
	~OrthographicCamera() = default;

	glm::mat4 getProjectionMatrix() const override;
};