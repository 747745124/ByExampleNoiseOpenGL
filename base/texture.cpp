#include <cassert>

#include "texture.h"
#include <iostream>

Texture::Texture()
{
	// create texture object
	glGenTextures(1, &_handle);
}

Texture::~Texture()
{
	// destroy texture object
	if (_handle != 0)
	{
		glDeleteTextures(1, &_handle);
		_handle = 0;
	}
}

void Texture::cleanup()
{
	if (_handle != 0)
	{
		glDeleteTextures(1, &_handle);
		_handle = 0;
	}
}

Texture2D::Texture2D(const std::string path) : _path(path)
{
	// load image to the memory
	stbi_set_flip_vertically_on_load(true);
	int width = 0, height = 0, channels = 0;
	unsigned char *data = stbi_load(_path.c_str(), &width, &height, &channels, 0);
	if (data == nullptr)
	{
		glDeleteTextures(1, &_handle);
		_handle = 0;
		throw std::runtime_error("load " + path + " failure");
	}

	// choose image format
	GLenum format = GL_RGB;
	switch (channels)
	{
	case 1:
		format = GL_RED;
		break;
	case 3:
		format = GL_RGB;
		break;
	case 4:
		format = GL_RGBA;
		break;
	default:
		cleanup();
		stbi_image_free(data);
		throw std::runtime_error("unsupported format");
	}

	// set texture parameters
	glBindTexture(GL_TEXTURE_2D, _handle);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);

	// transfer data to gpu
	// 1. set alignment for data transfer
	GLint alignment = 1;
	size_t pitch = width * channels * sizeof(unsigned char);
	if (pitch % 8 == 0)
		alignment = 8;
	else if (pitch % 4 == 0)
		alignment = 4;
	else if (pitch % 2 == 0)
		alignment = 2;
	else
		alignment = 1;

	glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);

	// 2. transfer data
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

	// 3. restore alignment
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	// 4. mipmap settings
	glGenerateMipmap(GL_TEXTURE_2D);


	// unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);

	// free data
	stbi_image_free(data);

	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		std::stringstream ss;
		ss << "texture object operation failure, (code " << error << ")";
		cleanup();
		throw std::runtime_error(ss.str());
	}
}

void Texture2D::bind() const
{
	glBindTexture(GL_TEXTURE_2D, _handle);
}

void Texture2D::unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

TextureCubemap::TextureCubemap(const std::vector<std::string> &filenames)
	: _paths(filenames)
{
	assert(filenames.size() == 6);
	// -----------------------------------------------
	// write your code to generate texture cubemap
	glBindTexture(GL_TEXTURE_CUBE_MAP, _handle);
	int width = 0, height = 0, channels = 0;

	for (unsigned int i = 0; i < _paths.size(); i++)
	{
		unsigned char *data = stbi_load(_paths[i].c_str(), &width, &height, &channels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::stringstream ss;
			ss << "Cubemap texture failed to load at path: " << _paths[i];
			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void TextureCubemap::bind() const
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, _handle);
}

void TextureCubemap::unbind() const
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

TextureShadow::TextureShadow()
{
	glBindTexture(GL_TEXTURE_2D, _handle);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32,
				 shadow_width, shadow_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _handle, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TextureShadow::bind() const
{
	glBindTexture(GL_TEXTURE_2D, _handle);
}
void TextureShadow::unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureShadow::draw()
{

	glViewport(0, 0, shadow_width, shadow_height);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

	glClear(GL_DEPTH_BUFFER_BIT);
}

CustomTextureShadow::CustomTextureShadow()
{
	glGenTextures(1, &custom_text);
	glGenFramebuffers(1, &depthMapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

	glGenRenderbuffers(1, &depthRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, shadow_width, shadow_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO);

	glBindTexture(GL_TEXTURE_2D, custom_text);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F,
				 shadow_width, shadow_height, 0, GL_RG, GL_FLOAT, NULL);

	// mipmap setting, not used
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// generate mipmap, problematic
	// glGenerateMipmap(GL_TEXTURE_2D);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, custom_text, 0);
	// glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, _handle, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		throw std::runtime_error("Framebuffer not complete");

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CustomTextureShadow::bind() const
{
	glBindTexture(GL_TEXTURE_2D, custom_text);
}

void CustomTextureShadow::unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}

void CustomTextureShadow::draw()
{
	glViewport(0, 0, shadow_width, shadow_height);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
