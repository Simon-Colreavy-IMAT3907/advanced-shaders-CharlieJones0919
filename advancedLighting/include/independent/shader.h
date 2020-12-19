#ifndef _SHADER_H_
#define _SHADER_H_
/**
\file shader.h
*/
#include <GLEW/glew.h>
#include <iterator>     
#include <string>       
#include <vector>
#include <iostream>
#include <fstream>

/**
\struct ShaderFile
\brief Stores data about a shader file.
*/
struct ShaderFile
{
	GLenum shaderType;    //!< What kind of shader this file is. (e.g. Vertex, fragment, geometry, etc...).
	const char* filePath; //!< File's directory address.

	//! Constructor to set the shader file's data values.
	/**
	\param type What to set the file's shaderType as.
	\param path What to set the file's filePath as.
	*/
	ShaderFile(GLenum type, const char* path) : shaderType(type), filePath(path) {}; 
};

/**
\class Shader
\brief An abstraction of a shader program.
*/
class Shader
{
public:
	GLuint programId; //!< Shader program's ID number for retrieval. 

	//! Shader constructor for a shader program that has vertex and fragment shaders.
	/**
	\param vertexPath Path to the shader program's vertex shader.
	\param fragPath Path to the shader program's fragment shader.
	*/
	Shader(const char* vertexPath, const char* fragPath) : programId(0)
	{
		std::vector<ShaderFile> fileVec;
		fileVec.push_back(ShaderFile(GL_VERTEX_SHADER, vertexPath));
		fileVec.push_back(ShaderFile(GL_FRAGMENT_SHADER, fragPath));
		loadFromFile(fileVec);
	}

	//! Shader constructor for a shader program that has vertex, fragment, AND geometry shaders.
	/**
	\param vertexPath Path to the shader program's vertex shader.
	\param fragPath Path to the shader program's fragment shader.
	\param geometryPath Path to the shader program's geometry shader.
	*/
	Shader(const char* vertexPath, const char* fragPath, const char* geometryPath) :programId(0)
	{
		std::vector<ShaderFile> fileVec;
		fileVec.push_back(ShaderFile(GL_VERTEX_SHADER, vertexPath));
		fileVec.push_back(ShaderFile(GL_FRAGMENT_SHADER, fragPath));
		fileVec.push_back(ShaderFile(GL_GEOMETRY_SHADER, geometryPath));
		loadFromFile(fileVec);
	}

	//! Shader deconstructor to delete the shader program.
	~Shader()
	{
		if (this->programId)
		{
			glDeleteProgram(this->programId);
		}
	}

	//! Set OpenGL to use this shader program.
	void use() const
	{
		glUseProgram(this->programId);
	}

private:
	//! Load the shader program's shader files.
	/**
	\param shaderFileVec Paths to the shader files.
	*/
	void loadFromFile(std::vector<ShaderFile>& shaderFileVec)
	{
		std::vector<GLuint> shaderObjectIdVec;
		std::string vertexSource, fragSource;
		std::vector<std::string> sourceVec;
		size_t shaderCount = shaderFileVec.size();

		//Read every given shader from its file path.
		for (size_t i = 0; i < shaderCount; ++i)
		{
			std::string shaderSource;
			if (!loadShaderSource(shaderFileVec[i].filePath, shaderSource))
			{
				std::cout << "Error::Shader could not load file:" << shaderFileVec[i].filePath << std::endl;
				return;
			}
			sourceVec.push_back(shaderSource);
		}
		bool bSuccess = true;

		//Create shader object from the given read files.
		for (size_t i = 0; i < shaderCount; ++i)
		{
			GLuint shaderId = glCreateShader(shaderFileVec[i].shaderType);
			const char *c_str = sourceVec[i].c_str();
			glShaderSource(shaderId, 1, &c_str, NULL);
			glCompileShader(shaderId);

			//Check the shader compiled successfully.
			GLint compileStatus = 0;
			glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus); 
			if (compileStatus == GL_FALSE) 
			{
				GLint maxLength = 0;
				glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);
				std::vector<GLchar> errLog(maxLength);
				glGetShaderInfoLog(shaderId, maxLength, &maxLength, &errLog[0]);
				std::cout << "Error::Shader file [" << shaderFileVec[i].filePath << " ] compiled failed," << &errLog[0] << std::endl;
				bSuccess = false;
			}

			shaderObjectIdVec.push_back(shaderId);
		}

		//If shader compilation was completed, create the shader program from these given shader objects
		if (bSuccess)
		{
			this->programId = glCreateProgram();
			for (size_t i = 0; i < shaderCount; ++i)
			{
				glAttachShader(this->programId, shaderObjectIdVec[i]);
			}

			glLinkProgram(this->programId);
			GLint linkStatus;
			glGetProgramiv(this->programId, GL_LINK_STATUS, &linkStatus);
			if (linkStatus == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetProgramiv(this->programId, GL_INFO_LOG_LENGTH, &maxLength);
				std::vector<GLchar> errLog(maxLength);
				glGetProgramInfoLog(this->programId, maxLength, &maxLength, &errLog[0]);
				std::cout << "Error::shader link failed," << &errLog[0] << std::endl;
			}
		}

		//Once shaders have been linked to the shader program, shader can be detached and deleted.
		for (size_t i = 0; i < shaderCount; ++i)
		{
			if (this->programId != 0)
			{
				glDetachShader(this->programId, shaderObjectIdVec[i]);
			}
			glDeleteShader(shaderObjectIdVec[i]);
		}
	}

	//! Get the shader's source code from its file.
	/**
	\param filePath Path to the shader source code file.
	\param source The code retrieved from this file.
	*/
	bool loadShaderSource(const char* filePath,std::string& source)
	{
		source.clear();

		std::ifstream in_stream(filePath);
		if (!in_stream)
		{
			return false;
		}

		source.assign(std::istreambuf_iterator<char>(in_stream), std::istreambuf_iterator<char>()); 
		return true;
	}
};
#endif