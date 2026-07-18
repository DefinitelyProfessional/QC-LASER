#pragma once

#include "imgui.h"

// for UIWindow 
#include "stage-utilities.hpp"

#include <filesystem>
#include <functional>
#include <vector>
#include <string>

namespace fs = std::filesystem;

// Sandbox selector window
class SandboxManagerWindow : public UIWindow {
private:
    fs::path saved_data_dir;
    std::string active_filename;
    std::vector<std::string> db_filenames;

    // UI State Trackers
    int selected_index = -1;
    char new_sandbox_input[64] = "";

    // Extract filename strings inside saved_data_dir
    inline std::vector<std::string> get_db_filenames() {
        std::vector<std::string> filenames;
        if (!fs::exists(saved_data_dir) || !fs::is_directory(saved_data_dir)) {
            throw std::invalid_argument("Sandbox data directory not found.");
        }

        for (const auto& entry : fs::directory_iterator(saved_data_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".db") {
                filenames.push_back(entry.path().filename().string());
            }
        }
        return filenames;
    }
public:
    // Event Listeners to be defined in main.cpp
    std::function<void(std::string)> Event_OnSelectSandbox;
    std::function<void(std::string)> Event_OnCreateSandbox;
    std::function<void(void)> Event_OnRefreshFilenames;

    // Constructor simply sets UIWindow window_name and directory filepath
    SandboxManagerWindow(const fs::path& data_dir, const std::string& active_file) : 
        UIWindow("Sandbox Manager"), saved_data_dir(data_dir), active_filename(active_file) {
            refresh_filenames();
        }
    
    // Refresh db_filenames
    void refresh_filenames() {
        db_filenames = get_db_filenames();
        selected_index = -1;
    }

    // Render function definition
    void Render() override {
        if (!is_open) return; // render control
        // Provide a sensible default size, but allow the user to resize it later
        ImGui::SetNextWindowSize(ImVec2(450, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin(window_name.c_str());

        ImGui::Text("Current Active Sandbox : %s", active_filename.c_str());
        ImGui::Text("Create New Sandbox");
        
        // Input box for the new file name
        ImGui::InputText("##NewFile", new_sandbox_input, sizeof(new_sandbox_input));
        ImGui::SameLine();
        
        // Only allow creation if the user actually typed something
        ImGui::BeginDisabled(strlen(new_sandbox_input) == 0);
        if (ImGui::Button("Create")) {
            if (Event_OnCreateSandbox) {
                Event_OnCreateSandbox(std::string(new_sandbox_input));
            }
            // Clear the input box after submission
            new_sandbox_input[0] = '\0'; 
        }
        ImGui::EndDisabled();

        ImGui::Separator();

        // Refresh Button aligned above the table
        if (ImGui::Button("Refresh List")) {
            refresh_filenames();
            if (Event_OnRefreshFilenames) Event_OnRefreshFilenames();
        }

        // ==========================================
        // MAIN PANEL: The Selectable File Table
        // ==========================================
        ImGui::Text("Available Databases:");

        // Table Flags: Draw borders, alternating row colors, and allow vertical scrolling.
        // We subtract space at the bottom to leave room for the load button.
        ImGuiTableFlags table_flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY;
        ImVec2 table_size = ImVec2(0.0f, -ImGui::GetFrameHeightWithSpacing() * 1.5f);

        if (ImGui::BeginTable("##SandboxTable", 1, table_flags, table_size)) {
            ImGui::TableSetupColumn("Filename");
            ImGui::TableHeadersRow();

            for (int i = 0; i < db_filenames.size(); i++) {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);

                bool is_selected = (selected_index == i);
                std::string& filename = db_filenames[i];

                // Draw the selectable item. SpanAllColumns ensures the whole row is clickable.
                if (ImGui::Selectable(filename.c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                    selected_index = i;
                }

                // Bonus Feature: Allow double-clicking a row to instantly load it
                if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    active_filename = filename;
                    if (Event_OnSelectSandbox) Event_OnSelectSandbox(active_filename);
                }
            }
            ImGui::EndTable();
        }

        ImGui::Separator();

        // ==========================================
        // BOTTOM PANEL: Confirmation Status
        // ==========================================
        std::string preview_text = (selected_index >= 0 && selected_index < db_filenames.size()) 
                                    ? db_filenames[selected_index] 
                                    : "No File Selected";
        
        ImGui::Text("Target: %s", preview_text.c_str());
        
        // Shove the final button to the far right side of the window
        ImGui::SameLine(ImGui::GetWindowWidth() - 120);

        // Disable the "Load" button if the user hasn't clicked a valid row yet
        ImGui::BeginDisabled(selected_index < 0);
        if (ImGui::Button("Load Sandbox", ImVec2(100, 0))) {
            active_filename = db_filenames[selected_index];
            if (Event_OnSelectSandbox) {
                Event_OnSelectSandbox(active_filename);
            }
        }
        ImGui::EndDisabled();
        ImGui::End();
    }
};