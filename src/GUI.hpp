#pragma once
#include "NoiseSynth.hpp"

void NoiseSynth::drawLightGUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const auto flags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings;

    if (!ImGui::Begin("Control Panel (Space to Hide)", nullptr, flags))
    {
        ImGui::End();
    }
    else
    {
        ImGui::Text("Synth Type");
        ImGui::RadioButton("Linear Mapping", &blendMode, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Variance Preserving Mapping", &blendMode, 1);
        ImGui::SameLine();
        ImGui::RadioButton("Use Gaussian Variance", &blendMode, 2);
        ImGui::Separator();
        ImGui::RadioButton("Source Texture", &blendMode, 3);
        ImGui::SameLine();
        ImGui::RadioButton("Gaussian Texture", &blendMode, 4);
        ImGui::End();

        ImGui::Separator();
        ImGui::Text("File name  (Press TAB to save, no extension required.)");
        ImGui::InputText("Screenshot Path",input_buffer, sizeof(input_buffer));

    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}