#pragma once
#include <memory>

#include <cstring>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include "../base/application.h"

typedef struct
{
	unsigned int vertex_index; //v
	unsigned int normal_index; //vn
	unsigned int uv_index;	   //vt
} a_index;

typedef struct
{
	a_index mesh_vec[3];
} a_mesh;

typedef struct
{
	std::vector<glm::vec3> _vertices; //v
	std::vector<glm::vec3> _normals;  //vn
	std::vector<glm::vec2> _uvs;	  //vt

} a_attrib;

typedef struct
{
	std::vector<a_mesh> _indices;
} a_shape;

const std::string export_path = "../export/";

bool obj_load(const char *filename, a_attrib *attrib, a_shape *shapes, glm::vec3 *min_vec, glm::vec3 *max_vec);

bool obj_unload(const char *filename, a_attrib attrib, a_shape shapes);
