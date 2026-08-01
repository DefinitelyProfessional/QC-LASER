#include "ui-utilities/ui-windows.hpp"
#include "ui-utilities/general-ui-utilities.hpp"

#include "imgui.h"

#include <windows.h>
#include <shellapi.h>

#include <filesystem>
#include <stdexcept>
#include <cstddef>
#include <vector>
#include <string>
#include <cfloat>

namespace fs = std::filesystem;
namespace UI {
// Constructor implementation
SandboxManagerWindow::SandboxManagerWindow(bool start_open, const fs::path& data_dir, std::string active_file) : 
UIWindow("SANDBOX MANAGER", start_open), saved_data_dir(data_dir), active_filename(active_file) {
    refresh_filenames();
}

// Private
bool SandboxManagerWindow::is_valid_new_filename(std::string& filename) {
    // filename guaranteed not empty by UI
    size_t first_dot = filename.find('.');
    if (first_dot != std::string::npos) {
        // If a dot exists, everything from that point to the end MUST be exactly ".h5"
        if (filename.compare(first_dot, std::string::npos, ".h5") != 0) {
            set_error_buffer(false, "Dot '.' in filenames must only be '.h5' extension.");
            return false;
        }
    // Mutation ONLY occurs here if no dot exists at all
    } else {filename += ".h5";}

    // Reject names missing a base filename (e.g., just ".h5")
    if (filename.length() <= 3) {
        set_error_buffer(false, "That's straight up an invalid filename.");
        return false;
    }

    // Replaces whitespace and checks for illegal characters simultaneously
    for (char& c : filename) {
        // Explicit ASCII bounds check (bypasses slow locale lookups of std::isalnum)
        bool is_valid_char = (
            (c >= 'a' && c <= 'z') || 
            (c >= 'A' && c <= 'Z') || 
            (c >= '0' && c <= '9') || 
            (c == '_' || c == '.')
        );
        if (!is_valid_char) {
            set_error_buffer(false, "Filename must only be Alphanumeric or '_' or '.'");
            return false;
        }
    }
    set_error_buffer(true, "");
    return true;
}

// Private
void SandboxManagerWindow::execute_delete_to_trash() {
    std::vector<std::wstring> target_paths;
    
    // Collect full paths of all ticked files
    for (int i = 0; i < db_filenames_size; i++) {
        if (selected_delete[i]) {
            fs::path full_path = saved_data_dir / db_filenames[i];
            target_paths.push_back(full_path.wstring());
        }
    }
    if (target_paths.empty()) return;

    // SHFileOperationW requires a double-null-terminated sequence of strings
    // Win32 API do get quirky, SHFILEOPSTRUCTW designed est Windows 95
    std::wstring multi_sz; // Construct the quirky input format
    for (const auto& path : target_paths) {
        multi_sz += path;
        multi_sz.push_back(L'\0');
    }
    multi_sz.push_back(L'\0'); // The required second null terminator as end

    SHFILEOPSTRUCTW file_op = {}; // Declare operation
    file_op.wFunc = FO_DELETE; // Commit delete file action
    file_op.pFrom = multi_sz.c_str(); // Pass the constructed quirky string
    // FOF_ALLOWUNDO is the flag that moves items to the Recycle Bin
    // FOF_NOCONFIRMATION disables the default Windows prompt
    file_op.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT | FOF_NOERRORUI;

    // System call execution, pass control to Windows File Explorer
    SHFileOperationW(&file_op);

    // Refresh the list after deletion
    refresh_filenames();
}

// Public
void SandboxManagerWindow::refresh_filenames() {
    db_filenames = std::vector<std::string>(); // Clean vector
    if (!fs::exists(saved_data_dir) || !fs::is_directory(saved_data_dir)) {
        throw std::invalid_argument("Sandbox data directory not found.");
    }
    // Populate db_filenames with currently present .h5 files
    for (const auto& entry : fs::directory_iterator(saved_data_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".h5") {
            db_filenames.push_back(entry.path().filename().string());
        }
    }
    // refresh variables
    db_filenames_size = db_filenames.size();
    selected_index = -1;
    // reset delete sellect system
    selected_delete.assign(db_filenames_size, false);
    selected_delete_count = 0;
    set_error_buffer(true, "");
}

// Public
void SandboxManagerWindow::Render() {
    // Window settings
    ImGui::SetNextWindowSize(ImVec2(370, 350), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_FirstUseEver);
    ImGui::Begin((">>> "+ window_name).c_str(), &is_open);

    // [TOP SECTION] Current Active and Create New
    ImGui::Text("Current Active Sandbox : ");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1.0f,0.5f,0.0f,1.0f), "%s", active_filename.c_str());
    ImGui::SameLine();
    ImGui::BeginDisabled(is_busy);
    if (ImGui::Button("Save###CurrentActiveSaveButton")) {
        if (EVENT_OnSaveCurrentSandbox) {
            EVENT_OnSaveCurrentSandbox();
            is_busy = true; // Main will reset this
        }
        refresh_filenames();
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
        ImGui::SetTooltip("Save the current active sandbox to disk");
    }
    ImGui::EndDisabled();
    ImGui::Separator();

    ImGui::Text("Create New Sandbox :");
    // Input box for the new file name
    bool create_text_entered = ImGui::InputTextWithHint(
        "###CreateNewInputText",
        "ex : new_sandbox.h5",
        new_sandbox_input,
        sizeof(new_sandbox_input),
        ImGuiInputTextFlags_CharsNoBlank |
        ImGuiInputTextFlags_EnterReturnsTrue
    );
    ImGui::SameLine();
    
    bool can_submit = (new_sandbox_input[0] != '\0' && !is_busy);

    ImGui::BeginDisabled(!can_submit);
    bool create_button_pressed = ImGui::Button("Create###CreateNewButton");
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal) && ImGui::BeginItemTooltip()) {
        ImGui::Text("Create a new sandbox file with that name,");
        ImGui::Text("creating existing files will only open said file");
        ImGui::EndTooltip();
    }
    ImGui::EndDisabled();

    if (can_submit && (create_text_entered || create_button_pressed)) {
        std::string filename = std::string(new_sandbox_input);
        if (is_valid_new_filename(filename)) {
            if (EVENT_OnCreateSandbox) {
                EVENT_OnCreateSandbox(filename);
                is_busy = true; // Main will reset this
            }
        }
        // Clear the input box after submission
        new_sandbox_input[0] = '\0';
    }
    // Error buffer display
    if (!error_buffer.empty()) {
        ImGui::TextColored(
            (success) ? ImVec4(0.2f,1.0f,0.1f,1.0f) : ImVec4(1.0f,0.0f,0.0f,1.0f), 
            "%s", error_buffer.c_str()
        );
    }
    ImGui::Separator();

    // TEMPORARY FPS READ
    ImGui::Text("FPS : %.1f", ImGui::GetIO().Framerate);

    // [MIDDLE SECTION] Selectable File Table and refresh file
    // Refresh Button aligned above the table
    if (ImGui::Button("Refresh###RefreshButton")) {refresh_filenames();}
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
        ImGui::SetTooltip("Refresh the sandbox file explorer table");
    }

    // Table as a simple file explorer
    if (ImGui::BeginTable("###SandboxFilesTable", 2, 
    ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY, 
    ImVec2(0.0f, -ImGui::GetFrameHeightWithSpacing() * 1.1f))) {
        // Freeze 0 columns and 1 row to keep header stationary
        ImGui::TableSetupScrollFreeze(0, 1);
        // Fixed-width column for the checkbox
        ImGui::TableSetupColumn("Del###TickBoxes",ImGuiTableColumnFlags_WidthFixed, 20.0f);
        ImGui::TableSetupColumn("###Available Sandboxes");
        
        // ImGui::TableHeadersRow();

        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted("Del"); 

        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted("  Available Sandboxes :");

        ImGuiListClipper clipper;
        clipper.Begin(db_filenames_size); 

        while (clipper.Step()) {
            for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                ImGui::TableNextRow();
                
                // Column 0: Checkbox
                ImGui::TableSetColumnIndex(0);
                
                // PushID is required in loops so ImGui can distinguish between checkboxes
                ImGui::PushID(i);
                bool is_checked = selected_delete[i];
                if (ImGui::Checkbox("###checkbox", &is_checked)) {
                    selected_delete[i] = is_checked;
                    // Update integer counter so we don't have to scan the vector every frame
                    selected_delete_count += (is_checked) ? 1 : -1;
                }
                ImGui::PopID();
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                    ImGui::SetTooltip("Select files to delete");
                }

                // Column 1: Filename
                ImGui::TableSetColumnIndex(1);
                const std::string& filename = db_filenames[i];

                if (ImGui::Selectable(filename.c_str(), selected_index == i,
                 ImGuiSelectableFlags_SpanAllColumns)) {
                    selected_index = i;
                }
                
                if ((ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) ||
                (ImGui::IsItemFocused() && ImGui::IsKeyPressed(ImGuiKey_Enter))) {
                    ImGui::BeginDisabled(is_busy);
                    selected_index = i;
                    if (EVENT_OnSelectSandbox) {
                        EVENT_OnSelectSandbox(db_filenames[selected_index]);
                        is_busy = true; // Main will reset this
                    }
                    ImGui::EndDisabled();
                }
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
                    ImGui::SetTooltip("Double click to select and load the file");
                }
            }
        }
        ImGui::EndTable();
    }
    ImGui::Separator();

    // Enable Delete button only if at least one checkbox is ticked
    ImGui::BeginDisabled(selected_delete_count == 0 || is_busy);
    if (ImGui::Button("Delete Selected File(s)###DeleteButton")) {
        ImGui::OpenPopup("Delete Confirmation"); // must match BeginPopupModal
    }
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
        ImGui::SetTooltip("Confirm to delete selected files");
    }
    ImGui::EndDisabled();

    // Delete Confirmation Popup 
    if (ImGui::BeginPopup("Delete Confirmation")) {
        ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f), "!!! DELETE FILE CONFIRMATION !!!");
        ImGui::Text("Confirm to move %d file(s) to the Recycle Bin", selected_delete_count);
        ImGui::Separator();
        
        ImGui::PushStyleColor(ImGuiCol_Button,        IM_COL32(220, 50, 50, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 70, 70, 255));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  IM_COL32(180, 30, 30, 255));
        ImGui::PushStyleColor(ImGuiCol_Text,          IM_COL32(255, 255, 255, 255));
        if (ImGui::Button("Delete###ConfirmDeleteButton", ImVec2(120, 0))) {
            execute_delete_to_trash();
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(4);

        ImGui::SetItemDefaultFocus(); // Allows pressing 'Enter' to cancel safely
        ImGui::SameLine();
        
        if (ImGui::Button("Cancel###CancelDeleteButton", ImVec2(-FLT_MIN, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::End();
}
}