#include "objLoader.h"

bool obj_load(const char* filename, a_attrib* attrib , a_shape* shapes, glm::vec3* min_vec, glm::vec3* max_vec) {
    std::ifstream file(filename);
    std::string line;
    std::string str;
    attrib->_vertices.clear();
    attrib->_normals.clear();
    attrib->_uvs.clear();
    //shapes->clear();
    std::vector<glm::vec3> _vertices; //v
    std::vector<float> _x; //x
    std::vector<float> _y; //y
    std::vector<float> _z; //z
    std::vector<glm::vec3> _normals;  //vn
    std::vector<glm::vec2> _uvs;	  //vt

    while (getline(file, line))
    {
        if (line.substr(0, 2) == "vt")
        {
            glm::vec2 uv;
            std::istringstream s(line.substr(3));

            s >> uv.x >> uv.y;

            _uvs.push_back(uv);

        }
        else if (line.substr(0, 2) == "vn")
        {
            glm::vec3 normal;
            std::istringstream s(line.substr(3));
 
            s >> normal.x >> normal.y >> normal.z;

         //   std::cout << normal.x << " " << normal.y<<" "<< normal.z << std::endl;
            _normals.push_back(normal);

        }
        else if (line.substr(0, 1) == "v")
        {
            glm::vec3 vertex;
            std::istringstream s(line.substr(2));


            s >> vertex.x >> vertex.y >> vertex.z;

            _x.push_back(vertex.x);
            _y.push_back(vertex.y);
            _z.push_back(vertex.z);
           _vertices.push_back(vertex);


        }
        else if (line.substr(0, 1) == "f")
        {
            //f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3 ...
            for (int k = 0; k < line.size(); k++) {
                if (line[k] == '/')
                    line[k] = ' ';
            }

            std::istringstream vtns(line.substr(2));


            a_mesh mesh;

            const std::string delim = "/";

            for (int i = 0; i < 3; i++) {
                vtns >> mesh.mesh_vec[i].vertex_index;
                vtns >> mesh.mesh_vec[i].uv_index;
                vtns >> mesh.mesh_vec[i].normal_index;
                //std::cout << mesh.mesh_vec[i].vertex_index << "/" << mesh.mesh_vec[i].uv_index << "/" << mesh.mesh_vec[i].normal_index << std::endl;

            }
         

            shapes->_indices.push_back(mesh);


        }
        else if (line.substr(0, 1) == "#")
        {

        }
        else
        {

        }
    }

    file.close();

    attrib->_vertices.swap(_vertices);
    attrib->_normals.swap(_normals);
    attrib->_uvs.swap( _uvs);



    max_vec->x = *std::max_element(_x.begin(), _x.end());
    max_vec->y = *std::max_element(_y.begin(), _y.end());
    max_vec->z = *std::max_element(_z.begin(), _z.end());

   
    min_vec->x = *std::min_element(_x.begin(), _x.end());
    min_vec->y = *std::min_element(_y.begin(), _y.end());
    min_vec->z = *std::min_element(_z.begin(), _z.end());

  
    return true;
}


bool obj_unload(const char* filename, a_attrib attrib, a_shape shapes) {
    std::ofstream file( export_path + filename);

    if (!file)
    {
        std::cout << "open fail" << std::endl;
        return false;
    }
    else
    {
       
        for (const auto& vec : attrib._vertices) {
            file << "v " << vec.x << " " << vec.y << " " << vec.z << std::endl;
        }

        for (const auto& uv : attrib._uvs) {
            file << "vt " << uv.x << " " << uv.y  << std::endl;
        }

        for (const auto& normal : attrib._normals) {
            file << "vn " << normal.x << " " << normal.y << " "<< normal.z << std::endl;
        }

        for (const auto& mesh : shapes._indices) {
            file << "f ";
            for (const auto& index : mesh.mesh_vec) {
                file << index.vertex_index << "/" << index.uv_index << "/" << index.normal_index << " ";
            }
            file << std::endl;
        }


        file.close();
    }

    return true;

}