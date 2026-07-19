#pragma once

#include "imgui.h"

// for UIWindow 
#include "general-utilities.hpp"

#include <filesystem>
#include <functional>
#include <vector>
#include <string>
#include <windows.h>
#include <shellapi.h>

namespace fs = std::filesystem;

// Sandbox selector window
class SandboxManagerWindow : public UIWindow {
private:
    fs::path saved_data_dir;
    const std::string& active_filename; // read-only reference from SandboxSessionManager
    std::vector<std::string> db_filenames;
    int db_filenames_size;
    
    // UI State Trackers
    std::vector<bool> db_checked;
    int selected_delete_count = 0; 
    int selected_index = -1;
    std::string error_buffer = "";
    char new_sandbox_input[64] = "";

    // Validate new sandbox filename
    bool is_valid_new_filename(std::string& filename) {
        // filename guaranteed not empty by UI
        size_t first_dot = filename.find('.');
        if (first_dot != std::string::npos) {
            // If a dot exists, everything from that point to the end MUST be exactly ".db"
            if (filename.compare(first_dot, std::string::npos, ".db") != 0) {
                error_buffer = "Dot '.' in filenames must only be '.db' extension.";
                return false;
            }
        // Mutation ONLY occurs here if no dot exists at all
        } else {filename += ".db";}

        // Reject names missing a base filename (e.g., just ".db")
        if (filename.length() <= 3) {
            error_buffer = "That's straight up an invalid filename.";
            return false;
        }

        // Replaces whitespace and checks for illegal characters simultaneously
        for (char& c : filename) {
            unsigned char uc = static_cast<unsigned char>(c);
            if (std::isspace(uc)) {c = '_';} 
            else {
                // Explicit ASCII bounds check (bypasses slow locale lookups of std::isalnum)
                bool is_valid_char = (c >= 'a' && c <= 'z') || 
                                    (c >= 'A' && c <= 'Z') || 
                                    (c >= '0' && c <= '9') || 
                                    c == '_' || c == '.';
                if (!is_valid_char) {
                    error_buffer = "Filename must only be Alphanumeric or '_' or '.'";
                    return false;
                }
            }
        }
        error_buffer = "";
        return true;
    }
    // Windows Shell API to move to Recycle Bin
    void execute_delete_to_trash() {
        std::vector<std::wstring> target_paths;
        
        // Collect full paths of all ticked files
        for (int i = 0; i < db_filenames_size; ++i) {
            if (db_checked[i]) {
                fs::path full_path = saved_data_dir / db_filenames[i];
                target_paths.push_back(full_path.wstring());
            }
        }

        if (target_paths.empty()) return;

        // SHFileOperationW requires a double-null-terminated sequence of strings
        std::wstring multi_sz;
        for (const auto& path : target_paths) {
            multi_sz += path;
            multi_sz.push_back(L'\0');
        }
        multi_sz.push_back(L'\0'); // Second null terminator

        SHFILEOPSTRUCTW file_op = {};
        file_op.wFunc = FO_DELETE;
        file_op.pFrom = multi_sz.c_str();
        // FOF_ALLOWUNDO is the flag that moves items to the Recycle Bin
        // FOF_NOCONFIRMATION disables the default Windows prompt (since we made our own)
        file_op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;

        SHFileOperationW(&file_op);

        // Refresh the list after deletion
        refresh_filenames();
    }

public:
    // Event Listeners to be defined in main.cpp
    std::function<void(std::string)> Event_OnSelectSandbox;
    std::function<void(std::string)> Event_OnCreateSandbox;
    std::function<void(void)> Event_OnSaveCurrentSandbox;

    // Constructor simply sets UIWindow window_name and directory filepath
    SandboxManagerWindow(const fs::path& data_dir, const std::string& active_file) : 
        UIWindow("Sandbox Manager"), saved_data_dir(data_dir), active_filename(active_file) {
            refresh_filenames();
        }
    
    // Extract filename strings inside saved_data_dir and refresh variables
    void refresh_filenames() {
        db_filenames = std::vector<std::string>(); // Clean vector
        if (!fs::exists(saved_data_dir) || !fs::is_directory(saved_data_dir)) {
            throw std::invalid_argument("Sandbox data directory not found.");
        }
        // Populate db_filenames with currently present .db files
        for (const auto& entry : fs::directory_iterator(saved_data_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".db") {
                db_filenames.push_back(entry.path().filename().string());
            }
        }
        // refresh variables
        db_filenames_size = db_filenames.size();
        selected_index = -1;
        // reset delete sellect system
        db_checked.assign(db_filenames_size, false);
        selected_delete_count = 0;
    }

    // Render function definition
    void Render() override {
        if (!is_open) return; // render control
        // Provide a sensible default size, but allow the user to resize it later
        ImGui::SetNextWindowSize(ImVec2(450, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin(window_name.c_str());

        // ==========================================
        // TOP PANEL Current Active and Create New
        // ==========================================
        ImGui::Text("Current Active Sandbox : ");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f,0.5f,0.0f,1.0f), "%s", active_filename.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Save")) {
            if (Event_OnSaveCurrentSandbox) {Event_OnSaveCurrentSandbox();}
            refresh_filenames();
        }
        ImGui::Separator();

        ImGui::Text("Enter New Sandbox Name :");
        if (!error_buffer.empty()) {
            ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f), "%s", error_buffer.c_str());
        }
        // Input box for the new file name
        ImGui::InputText("##NewFile", new_sandbox_input, sizeof(new_sandbox_input));
        ImGui::SameLine();
        
        // Only allow creation if the user actually typed something
        ImGui::BeginDisabled(new_sandbox_input[0] == '\0');
        if (ImGui::Button("Create")) {
            std::string filename = std::string(new_sandbox_input);
            if (is_valid_new_filename(filename)) {
                if (Event_OnCreateSandbox) {Event_OnCreateSandbox(filename);}
            }
            // Clear the input box after submission
            new_sandbox_input[0] = '\0';
        }
        ImGui::EndDisabled();

        ImGui::Separator();
        // ==========================================
        // MIDDLE PANEL Selectable File Table
        // ==========================================
        ImGui::Text("Double-click to select and load file.");

        // Refresh Button aligned above the table
        if (ImGui::Button("Refresh List")) {refresh_filenames();}

        ImGuiTableFlags table_flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY;
        ImVec2 table_size = ImVec2(0.0f, -ImGui::GetFrameHeightWithSpacing() * 1.0f);

        // Changed from 1 column to 2 columns
        if (ImGui::BeginTable("##SandboxTable", 2, table_flags, table_size)) {
            // Setup a narrow, fixed-width column for the checkbox
            ImGui::TableSetupColumn("##Tick", ImGuiTableColumnFlags_WidthFixed, 20.0f);
            ImGui::TableSetupColumn("Available Databases :");
            ImGui::TableHeadersRow();

            ImGuiListClipper clipper;
            clipper.Begin(db_filenames_size); 

            while (clipper.Step()) {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                    ImGui::TableNextRow();
                    
                    // Column 0: Checkbox
                    ImGui::TableSetColumnIndex(0);
                    
                    // PushID is required in loops so ImGui can distinguish between checkboxes
                    ImGui::PushID(i);
                    bool is_checked = db_checked[i];
                    if (ImGui::Checkbox("##chk", &is_checked)) {
                        db_checked[i] = is_checked;
                        // Update integer counter so we don't have to scan the vector every frame
                        selected_delete_count += (is_checked) ? 1 : -1;
                    }
                    ImGui::PopID();

                    // Column 1: Filename
                    ImGui::TableSetColumnIndex(1);
                    const std::string& filename = db_filenames[i];

                    if (ImGui::Selectable(filename.c_str(), selected_index == i)) {
                        selected_index = i;
                    }

                    if ((ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) ||
                        (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))) {
                        selected_index = i;
                        if (Event_OnSelectSandbox) Event_OnSelectSandbox(db_filenames[selected_index]);
                    }
                }
            }
            ImGui::EndTable();
        }
        ImGui::Separator();

        // Enable Delete button only if at least one checkbox is ticked
        ImGui::BeginDisabled(selected_delete_count == 0);
        if (ImGui::Button("Delete Selected File(s)")) {
            ImGui::OpenPopup("Delete Confirmation");
        }
        ImGui::EndDisabled();

        // The Confirmation Modal
        if (ImGui::BeginPopupModal("Delete Confirmation", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Confirm to move %d file(s) to the Recycle Bin", selected_delete_count);
            ImGui::Separator();
            
            if (ImGui::Button("Delete", ImVec2(120, 0))) {
                execute_delete_to_trash();
                ImGui::CloseCurrentPopup();
            }
            
            ImGui::SetItemDefaultFocus(); // Allows pressing 'Enter' to cancel safely
            ImGui::SameLine();
            
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::End();
    }
};