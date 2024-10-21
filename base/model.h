#pragma once

#include <string>
#include <vector>

#include <glad/glad.h>

#include "vertex.h"
#include "object3d.h"

#include "../src/utils/objLoader.h"

class Model : public Object3D
{
public:
	Model(const std::string &filepath, const std::string name);

	Model(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices);

	~Model();

	Model(Model &&model) noexcept = default;

	GLuint getVertexArrayObject() const;

	size_t getVertexCount() const;

	size_t getFaceCount() const;

	void draw() const;

	void export_model();

	glm::vec3 _min_vec;
	glm::vec3 _max_vec;

private:
	std::string _name;
	// vertices of the table represented in model's own coordinate
	std::vector<Vertex> _vertices;
	std::vector<uint32_t> _indices;

	// opengl objects
	GLuint _vao = 0;
	GLuint _vbo = 0;
	GLuint _ebo = 0;

	float Ka[3]; // ��Ӱɫ
	float Kd[3]; // ����ɫ
	float Ks[3]; // �߹�ɫ
	float Ke[3];

	std::string map_Kd; // ����������ͼ

	a_attrib _attrib;
	a_shape _shapes;

	void initGLResources();
};