#ifndef _TEXTURE_H_
#define _TEXTURE_H_
/**
\file texture.h
*/
#include <GLEW/glew.h>
#include <iostream>
#include <fstream>

/**
\class TextureHelper
\brief Loads textures from external files.
*/
class TextureHelper
{
public:
	//! A function to load a 2D texture from a file. If parameters aren't set they're set to their defaults.
	/**
	\param filename Name of the texture file.
	\param internalFormat Colour format of the texture.
	\param picFormat Picture format of the texture.
	\param loadChannels The texture's colour channels.
	\param alpha Whether or not to enable alpha transparency in the texture.
	*/
	static GLuint load2DTexture(const char* filename, GLint internalFormat = GL_RGB, GLenum picFormat = GL_RGB, int loadChannels = SOIL_LOAD_RGB, GLboolean alpha = false)
	{
		//Step 1: Create and bind textures.
		GLuint textureId = 0;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		//Step 2: Set wrapping data and clamp within edges.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, alpha ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, alpha ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	
		//Step 3: Set filtering data.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // 为MipMap设定filter方法
		
		//Variables to store data for next step.
		GLubyte *imageData = NULL;
		int picWidth, picHeight;
		int channels = 0;

		//Step 4: Load texture data from file.
		imageData = SOIL_load_image(filename, &picWidth, &picHeight, &channels, loadChannels);
		if (imageData == NULL) //Check that data was loaded successfully.
		{
			std::cerr << "Error::Texture could not load texture file:" << filename << std::endl;
			return 0;
		}
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, picWidth, picHeight, 0, picFormat, GL_UNSIGNED_BYTE, imageData);
		glGenerateMipmap(GL_TEXTURE_2D);
		
		//Step 4.5: Free imageData variable from memory.
		SOIL_free_image_data(imageData);

		//Step 5: Find the texture via OpenGL.
		glBindTexture(GL_TEXTURE_2D, 0);
		return textureId;
	}

	//! A function to create a texture which can be attached to a frame buffer.
	/**
	\param level Level of detail. More that 0 will reduce image detail.
	\param internalFormat Number of colour components in the texture.
	\param width Texture width.
	\param height Texture height.
	\param picFormat Format of pixel data.
	\param picDataType Type of pixel data.
	*/
	static GLuint makeAttachmentTexture(GLint level = 0, GLint internalFormat = GL_DEPTH24_STENCIL8, GLsizei width = 800, GLsizei height = 600, GLenum picFormat = GL_DEPTH_STENCIL, GLenum picDataType = GL_UNSIGNED_INT_24_8)
	{
		GLuint textId;
		glGenTextures(1, &textId);
		glBindTexture(GL_TEXTURE_2D, textId);
		glTexImage2D(GL_TEXTURE_2D, level, internalFormat, width, height, 0, picFormat, picDataType, NULL); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		return textId;
	}

	//! A function to create a multi-sampled texture which can be attached to a frame buffer.
	/**
	\param sampleNum Number of samples the texture is comprised of.
	\param internalFormat Texture's colour format.
	\param width Texture width.
	\param height Texture height.
	*/
	static GLuint makeMAAttachmentTexture(GLint samplesNum = 4, GLint internalFormat = GL_RGB, GLsizei width = 800, GLsizei height = 600)
	{
		GLuint textId;
		glGenTextures(1, &textId);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textId); 
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samplesNum, internalFormat, width, height, GL_TRUE);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

		return textId;
	}

#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

	//! Function to load an external DDS (DirectDraw Surface) file.
	/**
	\param filename Name of the DDS file.
	*/
	static GLuint loadDDS(const char * filename)
	{
		//Load the file from the given name.
		std::ifstream file(filename, std::ios::in | std::ios::binary);
		//Check file was loaded successfully.
		if (!file) {
			std::cout << "Error::loadDDs, could not open:" 
				<< filename << "for read." << std::endl;
			return 0;
		}

		//Get the file type.
		char filecode[4];
		file.read(filecode, 4);
		//Check the file type is a DDS.
		if (strncmp(filecode, "DDS ", 4) != 0) {
			std::cout << "Error::loadDDs, format is not dds :" << filename  << std::endl;
			file.close();
			return 0;
		}

		//Get surface data from file.
		char header[124];
		file.read(header, 124);

		//Set texture data to read surface data.
		unsigned int height = *(unsigned int*)&(header[8]);
		unsigned int width = *(unsigned int*)&(header[12]);
		unsigned int linearSize = *(unsigned int*)&(header[16]);
		unsigned int mipMapCount = *(unsigned int*)&(header[24]);
		unsigned int fourCC = *(unsigned int*)&(header[80]);

		//Variables for buffer.
		char * buffer = NULL;
		unsigned int bufsize;
		//Calculate required size of buffer from mipmap.
		bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
		buffer = new char[bufsize];
		file.read(buffer, bufsize);
		//Close DDS file.
		file.close();

		//Set component/format data.
		unsigned int components = (fourCC == FOURCC_DXT1) ? 3 : 4;
		unsigned int format;
		switch (fourCC)
		{
		case FOURCC_DXT1:
			format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
			break;
		case FOURCC_DXT3:
			format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			break;
		case FOURCC_DXT5:
			format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			break;
		default:
			delete[] buffer;
			return 0;
		}

		//Create OpenGL texture.
		GLuint textureID;
		glGenTextures(1, &textureID);

		//Bind the newly created texture to the GPU.
		glBindTexture(GL_TEXTURE_2D, textureID);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
		unsigned int offset = 0;

		//Load mipMaps.
		for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
		{
			unsigned int size = ((width + 3) / 4)*((height + 3) / 4)*blockSize;
			glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, buffer + offset);

			offset += size;
			width /= 2;
			height /= 2;

			//Constrain Non-Power-Of-Two textures. 
			if (width < 1) width = 1;
			if (height < 1) height = 1;

		}

		delete[] buffer;
		return textureID;
	}
};

#endif