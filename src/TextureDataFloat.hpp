#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include "../external/stb/stb_image.h"
#include "../base/texture.h"

//LUT structure
struct TextureDataFloat
{
	TextureDataFloat() : data(), width(0), height(0), channels(0) {}
	TextureDataFloat(const int w, const int h, const int c) :
		data(w * h * c), width(w), height(h), channels(c)
	{
	}

	float GetPixel(int w, int h, int c)
	{
		return data[h * width * channels + w * channels + c];
	}

	glm::vec3 GetColorAt(int w, int h)
	{
		return glm::vec3(
			data[h * width * channels + w * channels + 0],
			data[h * width * channels + w * channels + 1],
			data[h * width * channels + w * channels + 2]);
	}

	void SetPixel(int w, int h, int c, float value)
	{
		data[h * width * channels + w * channels + c] = value;
	}

	void SetColorAt(int w, int h, glm::vec3 value)
	{
		data[h * width * channels + w * channels + 0] = value.x;
		data[h * width * channels + w * channels + 1] = value.y;
		data[h * width * channels + w * channels + 2] = value.z;
	}

	std::vector<float> data;

	int width;
	int height;
	int channels;

	static int LoadTextureFromPNG(const char* filepath, TextureDataFloat& out) {
		int width, height, channels;
		unsigned char* image_data = stbi_load(filepath, &width, &height, &channels, 3); // Force 3 channels (RGB)
		
		std::cout<<"Loading Textures..."<<std::endl;

		if(channels !=3)
		{
			std::runtime_error ("Only RGB channels are supported");
			return 1;
		}

		if (!image_data) {
			std::runtime_error ("Couldn't load texture'");
			return 1;
		}

		out.height = height;
		out.width = width;
		out.channels = channels;

		// Create TextureDataFloat with the loaded dimensions and 3 channels
		out.data.resize((width) * (height) * 3);

		// Convert 8-bit data [0, 255] to float [0.0, 1.0]
		for (int i = 0; i < width * height * 3; ++i) {
			out.data[i] = static_cast<float>(image_data[i]) / 255.0f;
		}

		// Free the STB image data
		stbi_image_free(image_data);
		
		std::cout<<"Finish loading texture"<<std::endl;
		return 0;
	}
};

static void CreateGLTextureFromTextureDataStruct(Texture& texture, const TextureDataFloat& im, GLenum wrapMode, bool generateMips){

	if (im.data.empty())
	{
		std::runtime_error("Unable to create texture from empty texture data");
		return;
	}

	glBindTexture(GL_TEXTURE_2D, texture.getHandle());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		generateMips ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, im.width, im.height, 0,
				GL_RGB, GL_FLOAT, im.data.data());

	
	if (generateMips)
		glGenerateMipmap(GL_TEXTURE_2D);
	
	glBindTexture(GL_TEXTURE_2D, 0);
	return;
}