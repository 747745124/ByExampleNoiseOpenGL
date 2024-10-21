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
        ImGui::Text("Example");
        // ImGui::Text("Shadow Type");
        // ImGui::RadioButton("Direct Shadow Mapping", &shadowType, 0);
        // ImGui::SameLine();
        // ImGui::RadioButton("PCF", &shadowType, 1);
        // ImGui::SameLine();
        // ImGui::RadioButton("PCSS", &shadowType, 2);
        // ImGui::SameLine();
        // ImGui::RadioButton("VSM", &shadowType, 3);


        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}