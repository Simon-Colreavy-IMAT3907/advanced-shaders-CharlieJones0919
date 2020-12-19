#ifndef _MODEL_H_
#define _MODEL_H_
/**
\file model.h
*/
#include <map>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "mesh.h"
#include "texture.h"

/**
\class Model
\brief An abstraction of a renderable model comprised of one or multiple meshes.
*/
class Model
{
private:
	std::vector<Mesh> meshes;								  //!< This model's meshes.
	std::string modelFileDir;								  //!< Directory of the model file.
	typedef std::map<std::string, Texture> LoadedTextMapType; //!< Model's textures and their file directories.
	LoadedTextMapType loadedTextureMap;						  //!< Model's oaded textures.

	//! Processes mesh nodes.
	/**
	\param node The mesh nodes to process.
	\param sceneObjPtr Pointer to a scene object.
	*/
	bool processNode(const aiNode* node, const aiScene* sceneObjPtr)
	{
		//End if there aren't any nodes or scene objects to process.
		if (!node || !sceneObjPtr)
		{
			return false;
		}

		//Add every mesh in the given node to this model's meshes.
		for (size_t i = 0; i < node->mNumMeshes; ++i)
		{
			const aiMesh* meshPtr = sceneObjPtr->mMeshes[node->mMeshes[i]];
			if (meshPtr)
			{
				Mesh meshObj;
				if (this->processMesh(meshPtr, sceneObjPtr, meshObj))
				{
					this->meshes.push_back(meshObj);
				}
			}
		}

		//Also process the child nodes of the given node.
		for (size_t i = 0; i < node->mNumChildren; ++i)
		{
			this->processNode(node->mChildren[i], sceneObjPtr);
		}
		return true;
	}

	//! Set mesh data to a specified mesh.
	/**
	\param meshPtr The mesh to retrieve the data from.
	\param sceneObjPtr Pointer to a scene object.
	\param meshObj Pointer to the mesh object to set the meshPtr data to.
	*/
	bool processMesh(const aiMesh* meshPtr, const aiScene* sceneObjPtr, Mesh& meshObj)
	{
		//End if there aren't any meshes or scene objects to process.
		if (!meshPtr || !sceneObjPtr)
		{
			return false;
		}

		//Temporary variables to store mesh data in.
		std::vector<Vertex> vertData;
		std::vector<Texture> textures;
		std::vector<GLuint> indices;

		//Get above data from the mesh.
		for (size_t i = 0; i < meshPtr->mNumVertices; ++i)
		{
			Vertex vertex;

			//Get vertex positions from mesh.
			if (meshPtr->HasPositions())
			{
				vertex.position.x = meshPtr->mVertices[i].x;
				vertex.position.y = meshPtr->mVertices[i].y;
				vertex.position.z = meshPtr->mVertices[i].z;
			}

			//Get normals from mesh.
			if (meshPtr->HasNormals())
			{
				vertex.normal.x = meshPtr->mNormals[i].x;
				vertex.normal.y = meshPtr->mNormals[i].y;
				vertex.normal.z = meshPtr->mNormals[i].z;
			}

			//Get texture positions from mesh.
			if (meshPtr->HasTextureCoords(0))
			{
				vertex.texCoords.x = meshPtr->mTextureCoords[0][i].x;
				vertex.texCoords.y = meshPtr->mTextureCoords[0][i].y;
			}
			else
			{
				vertex.texCoords = glm::vec2(0.0f, 0.0f);
			}

		
			//Get tangents and bitangents from mesh.
			if (meshPtr->HasTangentsAndBitangents())
			{
				glm::vec3 tangent;
				tangent.x = meshPtr->mTangents[i].x;
				tangent.y = meshPtr->mTangents[i].y;
				tangent.z = meshPtr->mTangents[i].z;
				vertex.tangent = tangent;
			}

			vertData.push_back(vertex);
		}

		//Get indices from mesh.
		for (size_t i = 0; i < meshPtr->mNumFaces; ++i)
		{
			aiFace face = meshPtr->mFaces[i];
			if (face.mNumIndices != 3)
			{
				std::cerr << "Error:Model::processMesh, mesh not transformed to triangle mesh." << std::endl;
				return false;
			}
			for (size_t j = 0; j < face.mNumIndices; ++j)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		//Get texture data from mesh.
		if (meshPtr->mMaterialIndex >= 0)
		{
			const aiMaterial* materialPtr = sceneObjPtr->mMaterials[meshPtr->mMaterialIndex];

			//Get texture diffusion.
			std::vector<Texture> diffuseTexture;
			this->processMaterial(materialPtr, sceneObjPtr, aiTextureType_DIFFUSE, diffuseTexture);
			textures.insert(textures.end(), diffuseTexture.begin(), diffuseTexture.end());

			//Get texture specularity.
			std::vector<Texture> specularTexture;
			this->processMaterial(materialPtr, sceneObjPtr, aiTextureType_SPECULAR, specularTexture);
			textures.insert(textures.end(), specularTexture.begin(), specularTexture.end());

			//Get texture normal map data.
			std::vector<Texture> normalTexture;
			this->processMaterial(materialPtr, sceneObjPtr, aiTextureType_HEIGHT, normalTexture);
			textures.insert(textures.end(), normalTexture.begin(), normalTexture.end());
	
		//	//Get texture height map data.
		//	std::vector<Texture> heightTexture;
		//	this->processMaterial(materialPtr, sceneObjPtr, aiTextureType_HEIGHT, heightTexture);
		//	textures.insert(textures.end(), heightTexture.begin(), heightTexture.end());
		}

		//Set the retrieved data to the specified mesh object.
		meshObj.setData(vertData, textures, indices);
		return true;
	}

	//! Get the texture[s] from a given material.
	/**
	\param matPtr Pointer to the material to get the texture from.
	\param sceneObjPtr Pointer to a scene object.
	\param textureType The texture's type.
	\param textures Where to send the retrieved textures.
	*/
	bool processMaterial(const aiMaterial* matPtr, const aiScene* sceneObjPtr, const aiTextureType textureType, std::vector<Texture>& textures)
	{
		//Clear the current textures.
		textures.clear();

		//End if there aren't any materials or scene objects to process.
		if (!matPtr || !sceneObjPtr)
		{
			return false;
		}
		if (matPtr->GetTextureCount(textureType) <= 0)
		{
			return true;
		}

		//For every given material...
		for (size_t i = 0; i < matPtr->GetTextureCount(textureType); ++i)
		{
			Texture text; //Where to temporarily store collected texture data.
			aiString textPath;
			aiReturn retStatus = matPtr->GetTexture(textureType, i, &textPath);

			if ((retStatus != aiReturn_SUCCESS) || (textPath.length == 0))
			{
				std::cerr << "Warning, load texture type: " << textureType << " index: " << i << " failed with return value: " << retStatus << std::endl;
				continue;
			}

			std::string absolutePath = this->modelFileDir + "/" + textPath.C_Str();
			LoadedTextMapType::const_iterator it = this->loadedTextureMap.find(absolutePath);

			//If texture data was loaded successfully, set to texture.
			if (it == this->loadedTextureMap.end())
			{
				GLuint textId = TextureHelper::load2DTexture(absolutePath.c_str());
				text.id = textId;
				text.path = absolutePath;
				text.type = textureType;
				textures.push_back(text);
				loadedTextureMap[absolutePath] = text;
			}
			else
			{
				textures.push_back(it->second);
			}
		}
		return true;
	};
public:
	//! Draws the model to a shader.
	/**
	\param shader The shader to render the model to.
	*/
	void draw(const Shader& shader) const
	{
		//Draw all set meshes.
		for (std::vector<Mesh>::const_iterator it = this->meshes.begin(); this->meshes.end() != it; ++it)
		{
			it->draw(shader);
		}
	}

	//! Loads the model from an external file.
	/**
	\param filePath Directory to retrieve the model from.
	*/
	bool loadModel(const std::string& filePath)
	{
		Assimp::Importer importer;
		if (filePath.empty())
		{
			std::cerr << "Error:Model::loadModel, empty model file path." << std::endl;
			return false;
		}
		const aiScene* sceneObjPtr = importer.ReadFile(filePath, 
			aiProcess_Triangulate 
			| aiProcess_FlipUVs 
			| aiProcess_GenSmoothNormals
			| aiProcess_CalcTangentSpace);
		if (!sceneObjPtr
			|| sceneObjPtr->mFlags == AI_SCENE_FLAGS_INCOMPLETE
			|| !sceneObjPtr->mRootNode)
		{
			std::cerr << "Error:Model::loadModel, description: " 
				<< importer.GetErrorString() << std::endl;
			return false;
		}
		this->modelFileDir = filePath.substr(0, filePath.find_last_of('/')); 
		if (!this->processNode(sceneObjPtr->mRootNode, sceneObjPtr))
		{
			std::cerr << "Error:Model::loadModel, process node failed."<< std::endl;
			return false;
		}
		return true;
	}

	//! Deconstructor to remove all meshes.
	~Model()
	{
		for (std::vector<Mesh>::const_iterator it = this->meshes.begin(); this->meshes.end() != it; ++it)
		{
			it->final();
		}
	}

	//! Get function to retrieve the model's mesh[es].
	const std::vector<Mesh>& getMeshes() const { return this->meshes; }
};

#endif