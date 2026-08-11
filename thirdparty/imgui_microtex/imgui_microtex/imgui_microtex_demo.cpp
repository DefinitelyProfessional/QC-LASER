#include "imgui_microtex.h"
#include "imgui.h"

#include <exception>

namespace ImGuiMicroTeX {

void ShowDemoWindow() {
    if (!ImGui::Begin("ImGui MicroTeX Demo")) {
        ImGui::End();
        return;
    }

    // 1. Initialization Gate
    if (!ImGuiMicroTeX::IsInitialized()) {
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), 
            "Error: ImGuiMicroTeX is not initialized.");
        ImGui::TextWrapped("Ensure you have called ImGuiMicroTeX::Init() with valid file paths or buffers before rendering this window.");
        ImGui::End();
        return;
    }

    // 2. Cache & Engine Management
    if (ImGui::CollapsingHeader("Engine & Cache Diagnostics", ImGuiTreeNodeFlags_DefaultOpen)) {
        // Retrieve current cache size
        ImGui::Text("Cached Textures: %d", ImGuiMicroTeX::GetCacheSize());
        
        ImGui::SameLine();
        if (ImGui::Button("Clear Cache")) {
            ImGuiMicroTeX::ClearTextureCache();
        }

        // Eviction configuration
        static int evictionFrames = 60;
        ImGui::SetNextItemWidth(200.0f);
        if (ImGui::SliderInt("Eviction Frames", &evictionFrames, 0, 3600, "%d frames")) {
            ImGuiMicroTeX::SetEvictionFrames(evictionFrames);
        }
        ImGui::SameLine(); ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Set to 0 to disable eviction (textures live forever).\nDefault is 60 frames.");
        }
    }

    // 3. Interactive Playground
    if (ImGui::CollapsingHeader("Interactive Render Playground", ImGuiTreeNodeFlags_DefaultOpen)) {
        static char latexInput[1024] = 
            "f(x) = \\int_{-\\infty}^\\infty\n"
            "\\hat f(\\xi)\\,e^{2 \\pi i \\xi x}\n"
            "\\,d\\xi";
        
        static float fontSize = 32.0f;
        static ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // Default white
        static int styleIdx = 0;
        const char* styleNames[] = { "Display", "Text", "Script", "ScriptScript" };
        
        // Input Controls
        ImGui::InputTextMultiline("LaTeX Code", latexInput, sizeof(latexInput), ImVec2(0, 100));
        
        ImGui::SetNextItemWidth(300.0f);
        ImGui::SliderFloat("Font Size", &fontSize, 8.0f, 128.0f, "%.1f px");
        
        ImGui::SetNextItemWidth(300.0f);
        ImGui::ColorEdit4("Text Color", (float*)&color);
        
        ImGui::SetNextItemWidth(300.0f);
        ImGui::Combo("TeX Style", &styleIdx, styleNames, IM_ARRAYSIZE(styleNames));
        
        // Map UI combo index to the actual TexStyle enum
        ImGuiMicroTeX::TexStyle currentStyle = static_cast<ImGuiMicroTeX::TexStyle>(styleIdx);

        ImGui::Separator();
        ImGui::Text("Render Output:");
        
        // Render block with soft-fail safety
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.05f, 0.05f, 0.05f, 1.0f));
        ImGui::BeginChild("RenderArea", ImVec2(0, 180), true, ImGuiWindowFlags_HorizontalScrollbar);
        
        try {
            // Render to a cached GPU texture
            ImGuiMicroTeX::FormulaTexture tex = ImGuiMicroTeX::RenderToTexture(
                latexInput, fontSize, color, currentStyle
            );

            if (tex.TextureId() != 0) {
                // Display the texture
                ImGui::Image(tex.TextureId(), ImVec2((float)tex.Width, (float)tex.Height));
            }
        } catch (const std::exception& e) {
            // Catch parsing errors gracefully
            ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "Parser Error:");
            ImGui::TextWrapped("%s", e.what());
        }
        
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }

    // 4. Baseline Alignment Demonstration
    if (ImGui::CollapsingHeader("Inline Baseline Alignment", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextWrapped("This section demonstrates how to perfectly align ImGui standard text with MicroTeX images on the same typographic baseline.");
        ImGui::Spacing();

        ImGui::Text("Here is standard text");
        ImGui::SameLine();
        
        try {
            // Render inline math
            ImGuiMicroTeX::FormulaTexture inlineTex = ImGuiMicroTeX::RenderToTexture(
                "\\sum_{n=1}^{\\infty} \\frac{1}{n^2} = \\frac{\\pi^2}{6}", 
                ImGui::GetFontSize(), 
                ImGui::GetColorU32(ImGuiCol_Text), 
                ImGuiMicroTeX::TexStyle::Text
            );

            if (inlineTex.TextureId() != 0) {
                // Calculate typographic baseline offset
                
                // 1. Get the current font size
                float currentSize = ImGui::GetFontSize();

                // 2. Fetch the baked font metrics for this size, then get the Ascent
                float fontAscent = ImGui::GetFont()->GetFontBaked(currentSize)->Ascent;

                // 3. Calculate alignment
                float alignedY = ImGui::GetCursorPosY() + fontAscent - inlineTex.BaselineY;
                ImGui::SetCursorPosY(alignedY);
                
                ImGui::SetCursorPosY(alignedY);
                ImGui::Image(inlineTex.TextureId(), ImVec2((float)inlineTex.Width, (float)inlineTex.Height));
            }
        } catch (...) {
            ImGui::TextColored(ImVec4(1,0,0,1), "[Error]");
        }

        // Reset cursor Y back to standard alignment for the next text block
        ImGui::SameLine();
        float normalY = ImGui::GetCursorPosY(); 
        
        ImGui::SetCursorPosY(normalY); 
        ImGui::Text("and the sentence continues.");
    }

    ImGui::End();
}
}