#ifndef _MESH_H_
#define _MESH_H_
/**
\file mesh.h
*/
#include <GLEW/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>       
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "shader.h"

/**
\struct Vertex
\brief A structure to represet a single vertex element.
*/
struct Vertex
{
	glm::vec3 position;
	glm::vec2 texCoords;
	glm::vec3 normal;
	glm::vec3 tangent;
};

/**
\struct Texture
\brief A structure to represet a single OpenGL texture.
*/
struct Texture
{
	GLuint id;
	aiTextureType type;
	std::string path;
};

/**
\class Mesh
\brief Facilitates the creation, binding, unbinding and rendering of model meshes.
*/
class Mesh
{
private:
	std::vector<Vertex> vertData;	//!< Vertices in mesh.
	std::vector<GLuint> indices;	//!< Indicies in mesh.
	std::vector<Texture> textures;  //!< Textures for mesh.
	GLuint VAOId; //!< Vertex array.
	GLuint VBOId; //!< Vertex buffer object.
	GLuint EBOId; //!< Element buffer object.

	//! Initialise VAO, VBO and EBOs.
	void setupMesh()  
	{
		glGenVertexArrays(1, &this->VAOId);
		glGenBuffers(1, &this->VBOId);
		glGenBuffers(1, &this->EBOId);

		glBindVertexArray(this->VAOId);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBOId);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * this->vertData.size(), &this->vertData[0], GL_STATIC_DRAW);
		
		//Vertex positions.
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		//Vertex texture positions.
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(3 * sizeof(GL_FLOAT)));
		glEnableVertexAttribArray(1);

		//Vertex normal vectors.
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(5 * sizeof(GL_FLOAT)));
		glEnableVertexAttribArray(2);

		//Vertex tangent vectors.
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(8 * sizeof(GL_FLOAT)));
		glEnableVertexAttribArray(3);
		
		//Indicies data.
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBOId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)* this->indices.size(), &this->indices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
public:
	//! A constructor for creating a mesh.
	/**
	\params vertData Mesh vertices.
	\params textures Textures to set to the mesh.
	\params indices Mesh indices.
	*/
	Mesh(const std::vector<Vertex>& vertData, const std::vector<Texture> & textures, const std::vector<GLuint>& indices) :VAOId(0), VBOId(0), EBOId(0)
	{
		setData(vertData, textures, indices);
	}

	//! A constructor for creating a mesh with no data.
	Mesh() : VAOId(0), VBOId(0), EBOId(0) {};
	//! Default deconstructor.
	~Mesh() {};

	//! Function to set the data of an existing mesh.
	/**
	\params vertData Mesh vertices.
	\params textures Textures to set to the mesh.
	\params indices Mesh indices.
	*/
	void setData(const std::vector<Vertex>& vertData, const std::vector<Texture> & textures, const std::vector<GLuint>& indices)
	{
		this->vertData = vertData;
		this->indices = indices;
		this->textures = textures;
		if (!vertData.empty() && !indices.empty())
		{
			this->setupMesh();
		}
	}

	//! Clears VAO and buffers.
	void final() const
	{
		glDeleteVertexArrays(1, &this->VAOId);
		glDeleteBuffers(1, &this->VBOId);
		glDeleteBuffers(1, &this->EBOId);
	}

	//! Get vertex 
	GLuint getVAOId() const { return this->VAOId; }
	const std::vector<Vertex>& getVertices() const { return this->vertData; }
	const std::vector<GLuint>& getIndices() const { return this->indices; }

	//! Renders the mesh to a shader.
	/**
	\param shader The shader to render the mesh in.
	*/
	void draw(const Shader& shader) const 
	{
		//Check the array and buffer objects have been assigned.
		if ((VAOId == 0) || (VBOId == 0) || (EBOId == 0)) 
		{
			return;
		}

		//Bind the texture to the shader.
		glBindVertexArray(this->VAOId);
		int texUnitCnt = this->bindTextures(shader);

		//Draw the mesh.
		glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
		
		//Unbind the texture from the shader.
		glBindVertexArray(0);
		this->unBindTextures(texUnitCnt);
	}

	//! Binds the current VAOId to the shader.
	/**
	\param shader The shader to bind to.
	*/
	int bindTextures(const Shader& shader) const
	{
		//Temporary variables to store the texture values in.
		int diffuseCnt = 0, specularCnt = 0, texUnitCnt = 0,normalCnt = 0;
		
		//For all the added textures, add their data to their shader uniforms.
		for (std::vector<Texture>::const_iterator it = this->textures.begin(); this->textures.end() != it; ++it)
		{
			switch (it->type)
			{
				case aiTextureType_DIFFUSE:
				{
						glActiveTexture(GL_TEXTURE0 + texUnitCnt);
						glBindTexture(GL_TEXTURE_2D, it->id);
						std::stringstream samplerNameStr;
						samplerNameStr << "texture_diffuse" << diffuseCnt++;
						glUniform1i(glGetUniformLocation(shader.programId, samplerNameStr.str().c_str()), texUnitCnt++);
				}
				break;
				case aiTextureType_SPECULAR:
				{
					glActiveTexture(GL_TEXTURE0 + texUnitCnt);
					glBindTexture(GL_TEXTURE_2D, it->id);
					std::stringstream samplerNameStr;
					samplerNameStr << "texture_specular" << specularCnt++;
					glUniform1i(glGetUniformLocation(shader.programId, samplerNameStr.str().c_str()), texUnitCnt++);
				}
				break;
				case aiTextureType_HEIGHT:
				{
					glActiveTexture(GL_TEXTURE0 + texUnitCnt);
					glBindTexture(GL_TEXTURE_2D, it->id);
					std::stringstream samplerNameStr;
					samplerNameStr << "texture_normal" << normalCnt++;
					glUniform1i(glGetUniformLocation(shader.programId, samplerNameStr.str().c_str()), texUnitCnt++);
				}
				break;
			default:
				std::cerr << "Warning::Mesh::draw, texture type" << it->type
					<< " current not supported." << std::endl;
				break;
			}
		}
		return texUnitCnt;
	}

	//! Unbinds all textures.
	/**
	\param texUnitCnt How many textures were bound.
	*/
	void unBindTextures(const int texUnitCnt) const
	{
		for (int i = 0; i < texUnitCnt; ++i)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
};

#endif 