#ifndef MODEL_H
#define MODEL_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include "shader.h"
#include <iostream>
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
using namespace std;

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma);

class Model {
public:
	vector<Mesh*> meshes;
	string directory;
	bool gammaCorrection;
	int totalTriangles;

	Model() {};
	/*  Functions   */
	// constructor, expects a filepath to a 3D model.
	Model(string const &path, bool gamma = false) : gammaCorrection(gamma) {
		totalTriangles = 0;
		loadModel(path);
	}

	void loadModel(string const &path) {
		// read file via ASSIMP
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_JoinIdenticalVertices
												 | aiProcess_Triangulate
												 | aiProcess_GenSmoothNormals
												 //												 | aiProcess_FindInstances
												 | aiProcess_FlipUVs
												 | aiProcess_PreTransformVertices
		);
		//const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
		// check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			scene = importer.ReadFile(path, aiProcess_Triangulate
									  | aiProcess_GenSmoothNormals
									  | aiProcess_FindInstances
									  | aiProcess_FlipUVs
									  | aiProcess_PreTransformVertices
			);
			if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
			{
				cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
				return;
			}
		}
		if (scene->mNumTextures != 0)
			this->useTextured = true;
		else
			this->useTextured = false;

		// retrieve the directory path of the filepath
		directory = path.substr(0, path.find_last_of('/'));

		// process ASSIMP's root node recursively
		processNode(scene->mRootNode, scene);
	}

private:
	vector<Texture> textures_loaded;
	vector<Texture> textures;
	bool useTextured;
	// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode *node, const aiScene *scene) {
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++) {
			processNode(node->mChildren[i], scene);
		}
	}

	Mesh* processMesh(aiMesh *mesh, const aiScene *scene) {
		// data to fill
		vector<Vertex> vertices;
		vector<unsigned int> indices;

		// Walk through each of the mesh's vertices
		for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
			Vertex vertex;
			glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
							  // positions
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;
			// normals
			if (mesh->mNormals) {
				vector.x = mesh->mNormals[i].x;
				vector.y = mesh->mNormals[i].y;
				vector.z = mesh->mNormals[i].z;
				vertex.Normal = vector;
			}

			//if (mesh->mColors[0]) {
			//	vector.x = mesh->mColors[0][i][0];
			//	vector.y = mesh->mColors[0][i][1];
			//	vector.z = mesh->mColors[0][i][2];
			//	vertex.Color = vector;
			//}
			if (mesh->mTextureCoords[0]) {
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x;
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			} else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}
		totalTriangles += vertices.size() / 3;

		// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
		for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
			aiFace face = mesh->mFaces[i];
			// retrieve all indices of the face and store them in the indices vector
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		// process materials
		if (!this->useTextured) {
			int count = 0;
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			MeshMaterial meshMaterial;
			aiColor3D color(0.f, 0.f, 0.f);
			int tempInt;
			float tempFLoat;
			aiString testAiString;
			if (AI_SUCCESS == material->Get(AI_MATKEY_NAME, testAiString)) {
				meshMaterial.name = testAiString.data;
				++count;
			}
			if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
				meshMaterial.diffuse.x = color.r;
				meshMaterial.diffuse.y = color.g;
				meshMaterial.diffuse.z = color.b;
				++count;
			}
			if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_SPECULAR, color)) {
				meshMaterial.specular.x = color.r;
				meshMaterial.specular.y = color.g;
				meshMaterial.specular.z = color.b;
				++count;
			}

			if (AI_SUCCESS == material->Get(AI_MATKEY_SHADING_MODEL, tempInt)) {
				meshMaterial.shadingModel = tempInt;
				++count;
			}

			if (AI_SUCCESS == material->Get(AI_MATKEY_OPACITY, tempFLoat)) {
				meshMaterial.opacity = tempFLoat;
				++count;
			}
			if (AI_SUCCESS == material->Get(AI_MATKEY_SHININESS, tempFLoat)) {
				meshMaterial.shininess = tempFLoat;
				++count;
			}
			if (count != material->mNumProperties)
				cout << "Have unresolved properties" << endl;
			return new Mesh(vertices, indices, meshMaterial, textures);

		} else {
			if (mesh->mMaterialIndex >= 0) {
				aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
				vector<Texture> diffuseMaps = loadMaterialTextures(material,
																   aiTextureType_DIFFUSE, "texture_diffuse");
				textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
				vector<Texture> specularMaps = loadMaterialTextures(material,
																	aiTextureType_SPECULAR, "texture_specular");
				textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
			}
			return new Mesh(vertices, indices, MeshMaterial(), textures);

		}
	}

	vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName) {
		vector<Texture> textures;
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
			aiString str;
			mat->GetTexture(type, i, &str);
			bool skip = false;
			for (unsigned int j = 0; j < textures_loaded.size(); j++) {
				if (std::strcmp(textures_loaded[j].path.C_Str(), str.C_Str()) == 0) {
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}
			}
			if (!skip) {   // 如果纹理还没有被加载，则加载它
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), directory, gammaCorrection);
				texture.type = typeName;
				texture.path = str;
				textures.push_back(texture);
				textures_loaded.push_back(texture); // 添加到已加载的纹理中
			}
		}
		return textures;
	}
};

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false) {
	string filename = string(path);
	filename = directory + '/' + filename;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;

	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data) {
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	} else {
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

#endif