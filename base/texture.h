#pragma once

#include <string>
#include <sstream>
#include <vector>

#include <glad/glad.h>
#include "../external/stb/stb_image.h"

class Texture
{
public:
	Texture();
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;

	Texture(Texture&& other) noexcept {
        _handle = other._handle;
        other._handle = 0;  // so the old object won't call glDeleteTextures on it
    }

    Texture& operator=(Texture&& other) noexcept {
        if (this != &other) {
            cleanup(); // delete our old one if valid
            _handle = other._handle;
            other._handle = 0;
        }
        return *this;
    }

	virtual ~Texture();

	virtual void bind() const = 0;

	virtual void unbind() const = 0;

	virtual GLuint getHandle() const {
		return _handle;
	};

protected:
	GLuint _handle = {};

	virtual void cleanup();
};

class Texture2D : public Texture
{
public:
	Texture2D(const std::string path);
	Texture2D(Texture2D&&) = default;
    Texture2D& operator=(Texture2D&&) = default;

	Texture2D() = default;

	~Texture2D() = default;

	void bind() const override;

	virtual void unbind() const;

private:
	std::string _path;
};

class TextureCubemap : public Texture
{
public:
	TextureCubemap(const std::vector<std::string> &filenames);

	~TextureCubemap() = default;

	void bind() const override;

	void unbind() const override;

private:
	std::vector<std::string> _paths;
};

class TextureShadow : public Texture
{
public:
	TextureShadow();

	~TextureShadow() = default;

	void draw();

	void bind() const override;

	void unbind() const override;

private:
	GLuint shadow_width = 2000;
	GLuint shadow_height = 1200;
	GLuint depthMapFBO;
	// std::string _path;
};

class CustomTextureShadow : public Texture
{
public:
	CustomTextureShadow();

	~CustomTextureShadow() = default;

	void draw();

	void bind() const override;

	void unbind() const override;

private:
	GLuint shadow_width = 2000;
	GLuint shadow_height = 1200;
	GLuint depthMapFBO;
	GLuint depthRBO;
	GLuint custom_text;
	// std::string _path;
};
