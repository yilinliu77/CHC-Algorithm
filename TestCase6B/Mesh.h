#ifndef MESH_H
#define MESH_H

#include <iostream>
#include <vector>
#include "shader.h"
#include "GL/glew.h"
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include <algorithm>
#include <assimp/types.h>
using namespace std;

struct Bounds3f {
	glm::vec3 pMin, pMax;

	Bounds3f() : pMax(glm::vec3(0)), pMin(glm::vec3(0)) {}
	Bounds3f(const glm::vec3 p) : pMin(p), pMax(p) {}

	Bounds3f(const glm::vec3 p1, const glm::vec3 p2)
		: pMin(glm::vec3(std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z)))
		, pMax(glm::vec3(std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z))) {}

	Bounds3f unionBounds(const Bounds3f b) {
		if (pMax == glm::vec3(0) && pMin == glm::vec3(0))
			return b;
		return Bounds3f(
			glm::vec3(
				std::min(b.pMin.x, pMin.x),
				std::min(b.pMin.y, pMin.y),
				std::min(b.pMin.z, pMin.z))
			, glm::vec3(
				std::max(b.pMax.x, pMax.x),
				std::max(b.pMax.y, pMax.y),
				std::max(b.pMax.z, pMax.z)));
	}

	int MaximumExtent() const {
		glm::vec3 d = pMax - pMin;
		if (d.x > d.y && d.x > d.z)
			return 0;
		else if (d.y > d.z)
			return 1;
		else
			return 2;
	}

	glm::vec3 Offset(const glm::vec3& p) const {
		glm::vec3 o = p - pMin;
		if (pMax.x > pMin.x) o.x /= pMax.x - pMin.x;
		if (pMax.y > pMin.y) o.y /= pMax.y - pMin.y;
		if (pMax.z > pMin.z) o.z /= pMax.z - pMin.z;
		return o;
	}

	glm::vec3 getCentroid() {
		return (pMin + pMax) * 0.5f;
	}

	float SurfaceArea() const {
		glm::vec3 d = pMax - pMin;
		return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
	}

	//WARNING:just square
	float* getVertexData() {
		float edge = pMax[0] - pMin[0];

	}
};

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct Texture {
	unsigned int id;
	string type;
	aiString path;
};

struct MeshMaterial {
	glm::vec3 diffuse = glm::vec3(1.0f);
	glm::vec3 specular = glm::vec3(1.0f);
	float shininess = 1;
	int shadingModel = 0;
	float opacity = 1;
	int wireframe = 0;
	string name = "";
};

class Mesh {
public:
	/*  Mesh Data  */
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	MeshMaterial material;
	vector<Texture> textures;
	unsigned int VAO;

	/*  Functions  */
	// constructor
	Mesh(vector<Vertex> vertices, vector<unsigned int> indices
		 , MeshMaterial material, vector<Texture> textures) {
		this->vertices = vertices;
		this->indices = indices;
		this->material = material;

		// now that we have all the required data, set the vertex buffers and its attribute pointers.
		setupMesh();

		//deal with bounds
		float minx = 99999999, miny = 99999999, minz = 99999999;
		float maxx = -99999999, maxy = -99999999, maxz = -99999999;
		int countTemp = vertices.size();
		for (int i = 0; i < countTemp; ++i) {
			minx = vertices[i].Position.x < minx ? vertices[i].Position.x : minx;
			miny = vertices[i].Position.y < miny ? vertices[i].Position.y : miny;
			minz = vertices[i].Position.z < minz ? vertices[i].Position.z : minz;
			maxx = vertices[i].Position.x > maxx ? vertices[i].Position.x : maxx;
			maxy = vertices[i].Position.y > maxy ? vertices[i].Position.y : maxy;
			maxz = vertices[i].Position.z > maxz ? vertices[i].Position.z : maxz;

		}
		this->bounds = Bounds3f(glm::vec3(minx, miny, minz), glm::vec3(maxx, maxy, maxz));
	}

	Mesh(glm::vec3 c, float edge) {
		glm::vec3 boundVerticesVector[] = {
			c + glm::vec3(edge / 2, edge / 2, edge / 2),
			glm::vec3(1, 1, 1),
			c + glm::vec3(edge / 2, -edge / 2, edge / 2),
			glm::vec3(1, -1, 1),
			c + glm::vec3(edge / 2, -edge / 2, -edge / 2),
			glm::vec3(1, -1, -1),
			c + glm::vec3(edge / 2, edge / 2, -edge / 2),
			glm::vec3(1, 1, -1),
			c + glm::vec3(-edge / 2, -edge / 2, -edge / 2),
			glm::vec3(-1, -1, -1),
			c + glm::vec3(-edge / 2, -edge / 2, edge / 2),
			glm::vec3(-1, -1, 1),
			c + glm::vec3(-edge / 2, edge / 2, edge / 2),
			glm::vec3(-1, 1, 1),
			c + glm::vec3(-edge / 2, edge / 2, -edge / 2),
			glm::vec3(-1, 1, -1)
		};
		Vertex vertex;
		for (int i = 0; i < 8; ++i) {
			vertex.Position = boundVerticesVector[2 * i];
			vertex.Normal = boundVerticesVector[2 * i + 1];
			this->vertices.push_back(vertex);
		}
		for (int i = 0; i < 36; i++)
			this->indices.push_back(boundIndex[i]);
		this->material = MeshMaterial();
		this->material.diffuse = glm::vec3(0, 1, 0);
		this->bounds = Bounds3f(c + glm::vec3(-edge / 2, -edge / 2, -edge / 2)
								, c + glm::vec3(edge / 2, edge / 2, edge / 2));

		setupMesh();

	}

	// render the mesh
	void Draw(Shader* shader) {
		if (this->textures.empty()) {
			shader->setVec3("material.diffuse0", this->material.diffuse);
			shader->setVec3("material.specular0", this->material.specular);
			shader->setFloat("material.shininess0", this->material.shininess);
		} else {
			unsigned int diffuseNr = 1;
			unsigned int specularNr = 1;
			for (unsigned int i = 0; i < textures.size(); i++) {
				glActiveTexture(GL_TEXTURE0 + i); // 在绑定之前激活相应的纹理单元
												  // 获取纹理序号（diffuse_textureN 中的 N）
				stringstream ss;
				string number;
				string name = textures[i].type;
				if (name == "material.diffuse")
					ss << diffuseNr++; // 将 unsigned int 插入到流中
				else if (name == "material.specular")
					ss << specularNr++; // 将 unsigned int 插入到流中
				number = ss.str();

				shader->setFloat(("material." + name + number).c_str(), i);
				glBindTexture(GL_TEXTURE_2D, textures[i].id);
			}
			glActiveTexture(GL_TEXTURE0);
		}
		// draw mesh
		glBindVertexArray(this->VAO);
		glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, 2);
		glBindVertexArray(0);

	}


	Bounds3f getBounds() { return this->bounds; }
	//glm::mat4 getModel() { return this->model; }
	glm::vec3 getCentroid() { return this->bounds.getCentroid(); }


private:
	/*  Render data  */
	unsigned int VBO, EBO;
	Bounds3f bounds;
	static GLuint boundIndex[36];
	/*  Functions    */
	// initializes all the buffer objects/arrays
	void setupMesh() {
		// create buffers/arrays
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);
		// load data into vertex buffers
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// A great thing about structs is that their memory layout is sequential for all its items.
		// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
		// again translates to 3/2 floats which translates to a byte array.
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		// set the vertex attribute pointers
		// vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		// vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		// color
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));


		glBindVertexArray(0);
	}
};

GLuint Mesh::boundIndex[36] = {
	0, 1, 2, 0, 2, 3, 0, 5, 1, 0, 6, 5, 0, 3, 7, 0, 7, 6, 4, 5, 6, 4, 6, 7, 4, 2, 3, 4, 3, 7, 4, 5, 1, 4, 1, 2
};

#endif
