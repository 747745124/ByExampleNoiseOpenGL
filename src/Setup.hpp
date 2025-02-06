#pragma once
#include "NoiseSynth.hpp"
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
    _synthShader->setInt("inv_lut_texture",2);

    TextureDataFloat _noiseTextureData;
    if(TextureDataFloat::LoadTextureFromPNG(noiseTexturePath.c_str(),_noiseTextureData))
        return;
    
    // note that to perform an approximate OT through histogram normalization
    // a decorrelation step is required (i.e. PCA)
    TextureDataFloat input_decorrelated = TextureDataFloat(_noiseTextureData.width, _noiseTextureData.height, 3);

	DecorrelateColorSpace(_noiseTextureData, 
							input_decorrelated, 
							this->colorSpaceVec1, 
							this->colorSpaceVec2, 
							this->colorSpaceVec3, 
							this->colorSpaceOrigin);

    _synthShader->use();
    _synthShader->setVec3("_colorSpaceVec1",this->colorSpaceVec1);
    _synthShader->setVec3("_colorSpaceVec2",this->colorSpaceVec2);
    _synthShader->setVec3("_colorSpaceVec3",this->colorSpaceVec3);
    _synthShader->setVec3("_colorSpaceOrigin",this->colorSpaceOrigin);
    
    //calculating inverse transformation
    const int LUT_WIDTH = 128;
    auto Tinv = TextureDataFloat(LUT_WIDTH, 1, 3);
    for(int channel = 0 ; channel < 3 ; channel++)
	{
		ComputeinvT(input_decorrelated, Tinv, channel);
	}

    _invLutTexture.reset(new Texture2D());
    _noiseTexture.reset(new Texture2D(noiseTexturePath));
    _gaussianTexture.reset(new Texture2D(gaussianTexturePath));

    CreateGLTextureFromTextureDataStruct(*_invLutTexture.get(), Tinv, GL_CLAMP_TO_EDGE, false);

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

