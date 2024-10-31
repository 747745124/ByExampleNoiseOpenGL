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


const int fps = 40;

class SceneRoaming : public Application
{
public:
	SceneRoaming(const std::string &basedir);

	~SceneRoaming();

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


	void renderFrame() override;

	void drawLightGUI();

	void draw_debug_pass();

	void draw_blend_pass();


#ifdef _win32
	// Screenshot
	bool Screenshot();
#endif
};

//synth shader
const std::string synthTexturePath = "../data/noise/granite_256.png";
const std::string gaussianTexturePath = "../output/granite_256_g.png";
const std::string synthVertCode = "../shader/synth.vs";
const std::string synthFragCode = "../shader/synth.fs";
const std::string debugVertCode = "../shader/debug.vs";
const std::string debugFragCode = "../shader/debug.fs";



