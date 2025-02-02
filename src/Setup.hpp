#pragma once
#include "NoiseSynth.h"
#include "Precompute.hpp"
NoiseSynth::NoiseSynth(const std::string &basedir)
{   
 
#ifdef DEBUG
    cout << "Model Initialized" << endl;
#endif

    // init render quad function
    rq.reset(new RenderQuad);

    //synth shader initialization
    _synthShader.reset(new Shader(synthVertCode, synthFragCode));
    _debugShader.reset(new Shader(debugVertCode, debugFragCode));
    _debugShader->use();
    _debugShader->setInt("debugText", 0);

    _synthShader->use();
    _synthShader->setInt("src_texture",0);
    _synthShader->setInt("gauss_texture",1);


    TextureDataFloat _noiseTextureData;
    if(TextureDataFloat::LoadTextureFromPNG(noiseTexturePath.c_str(),_noiseTextureData))
        return;

    //calculating inverse transformation
    const int LUT_WIDTH = 128;
    auto Tinv = TextureDataFloat(LUT_WIDTH, 1, 3);
    for(int channel = 0 ; channel < 3 ; channel++)
	{
		ComputeinvT(_noiseTextureData, Tinv, channel);
	}

    Tinv_id = CreateGLTextureFromTextureDataStruct(Tinv, GL_CLAMP_TO_EDGE, false);

    for(int channel = 0 ; channel < 3 ; channel++)
	{
		PrefilterLUT(_noiseTextureData, Tinv, channel);
	}

    //synth noise textures
    _noiseTexture.reset(new Texture2D(noiseTexturePath));
    _gaussianTexture.reset(new Texture2D(gaussianTexturePath));

    // init imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init();
}

NoiseSynth::~NoiseSynth()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

