#pragma once
#include "SceneRoaming.h"
SceneRoaming::SceneRoaming(const std::string &basedir)
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

    //synth noise textures
    _noiseTexture.reset(new Texture2D(synthTexturePath));

    // init imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init();

}

SceneRoaming::~SceneRoaming()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
