#pragma once

#include <memory>
#include <string>
#include <random>
#include "../external/imgui/imgui.h"
#include "../base/application.h"
#include "../base/shader.h"
#include "../base/model.h"
#include "../base/light.h"
#include "../base/texture.h"
#include "../base/camera.h"
#include "../base/skybox.h"
#include "RenderQuad.h"

//HERE ARE THE PATH TO CHANGE, one is input, one is gaussianized input
const std::string noiseTexturePath = "../data/noise/granite_256.png";
const std::string gaussianTexturePath = "../gaussian_output/granite_256_g.png";

//Synth shader
const std::string synthVertCode = "../shader/synth.vs";
const std::string synthFragCode = "../shader/synth.fs";
const std::string debugVertCode = "../shader/debug.vs";
const std::string debugFragCode = "../shader/debug.fs";
const int fps = 40;

class NoiseSynth : public Application
{
public:
	NoiseSynth(const std::string &basedir);

	~NoiseSynth();

	std::unique_ptr<RenderQuad> rq;

private:

	void handleInput() override;
	
	std::unique_ptr<Shader> _debugShader;
	//synth shader
	std::unique_ptr<Shader> _synthShader;
	//synth noise texture
	std::unique_ptr<Texture> _noiseTexture;
	//synth noise texture
	std::unique_ptr<Texture> _gaussianTexture;
	//inv_T transformation
	std::unique_ptr<Texture> _invLutTexture;

	//decorrelation related variables
	glm::vec3 colorSpaceVec1;
	glm::vec3 colorSpaceVec2;
	glm::vec3 colorSpaceVec3;
	glm::vec3 colorSpaceOrigin;

	//GUI
	int blendMode = 0;
	bool hideGUI = false;
	char input_buffer[256] = "";

	void renderFrame() override;

	void drawLightGUI();

	void draw_debug_pass();

	void draw_blend_pass();

#ifdef __APPLE__
	// Captures the current OpenGL framebuffer and saves it to a file
    bool saveScreenshot(const std::string& filename, int width, int height);
#endif
};
