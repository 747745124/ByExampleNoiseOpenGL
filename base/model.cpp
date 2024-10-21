#include <iostream>
#include <unordered_map>

#include "../external/tiny_obj_loader/tiny_obj_loader.h"
#include "model.h"

Model::Model(const std::string &filepath, const std::string name)
{

	a_attrib attrib;
	a_shape shapes;

	_name = name;

	glm::vec3 min_vec = {0.0f, 0.0f, 0.0f};
	glm::vec3 max_vec = {0.0f, 0.0f, 0.0f};

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::unordered_map<Vertex, uint32_t> uniqueVertices;

	if (!obj_load(filepath.c_str(), &attrib, &shapes, &min_vec, &max_vec))
	{
		throw std::runtime_error("load " + filepath + " failure");
	}

	for (const auto &mesh : shapes._indices)
	{
		for (const auto &index : mesh.mesh_vec)
		{
			Vertex vertex{};

			int _v = index.vertex_index;
			int _vt = index.uv_index;
			int _vn = index.normal_index;

			vertex.position = attrib._vertices[_v - 1];

			if (_vt >= 0)
			{
				vertex.texCoord = attrib._uvs[_vt - 1];
			}

			if (_vn >= 0)
			{
				vertex.normal = attrib._normals[_vn - 1];
			}

			// check if the vertex appeared before to reduce redundant data
			if (uniqueVertices.count(vertex) == 0)
			{
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}

	_vertices = vertices;
	_indices = indices;

	_min_vec = min_vec;
	_max_vec = max_vec;

	_attrib = attrib;
	_shapes = shapes;

	initGLResources();
}

Model::Model(const std::vector<Vertex> &vertices, const std::vector<uint32_t> &indices)
	: _vertices(vertices), _indices(indices)
{
	initGLResources();
}

Model::~Model()
{
	if (_ebo != 0)
	{
		glDeleteBuffers(1, &_ebo);
		_ebo = 0;
	}

	if (_vbo != 0)
	{
		glDeleteBuffers(1, &_vbo);
		_vbo = 0;
	}

	if (_vao != 0)
	{
		glDeleteVertexArrays(1, &_vao);
		_vao = 0;
	}
}

void Model::draw() const
{
	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)_indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Model::export_model()
{

	if (obj_unload(_name.c_str(), _attrib, _shapes))
	{
		std::cout << "export success" << std::endl;
	}
}

GLuint Model::getVertexArrayObject() const
{
	return _vao;
}

size_t Model::getVertexCount() const
{
	return _vertices.size();
}

size_t Model::getFaceCount() const
{
	return _indices.size() / 3;
}

void Model::initGLResources()
{
	// create a vertex array object
	glGenVertexArrays(1, &_vao);
	// create a vertex buffer object
	glGenBuffers(1, &_vbo);
	// create a element array buffer
	glGenBuffers(1, &_ebo);

	glBindVertexArray(_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * _vertices.size(), _vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(uint32_t), _indices.data(), GL_STATIC_DRAW);

	// specify layout, size of a vertex, data type, normalize, sizeof vertex array, offset of the attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, normal));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, texCoord));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}