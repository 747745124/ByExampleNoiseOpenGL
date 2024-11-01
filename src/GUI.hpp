#pragma once
#include "SceneRoaming.h"

void SceneRoaming::drawLightGUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    const auto flags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings;

    if (!ImGui::Begin("Control Panel", nullptr, flags))
    {
        ImGui::End();
    }
    else
    {
        ImGui::Text("Shadow Type");
        ImGui::RadioButton("Linear Mapping", &blendMode, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Variance Preserving Mapping", &blendMode, 1);
        ImGui::SameLine();
        ImGui::RadioButton("Use Gaussian Variance", &blendMode, 2);
        ImGui::End();

        ImGui::Separator();
        ImGui::Text("File name");
        ImGui::InputText("Screenshot Path",input_buffer, sizeof(input_buffer));

    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}