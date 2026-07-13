// File: src/headers/ui-utilities/general-ui-utilities.hpp
#pragma once
#include "imgui.h"
#include <functional>
#include <string>
#include <cstring> // Required for text buffer manipulation

namespace UI {
    
    class UI_ModuleName_Controller {
    public:
        // =========================================================================
        // 1. EVENT LISTENERS (The "Slots")
        // These are placeholders for functions. The UI doesn't know what these do; 
        // it just knows to call them when a specific ImGui widget is interacted with.
        // =========================================================================
        
        // A basic trigger event (no data passed)
        std::function<void()> Event_OnSimpleActionTriggered;
        
        // An event that passes a specific value back to the engine
        std::function<void(int)> Event_OnDataActionTriggered;
        
        // An event that triggers immediately when a value changes state
        std::function<void(bool)> Event_OnToggleStateChanged;

        // =========================================================================
        // 2. INTERNAL UI STATE (The "Memory")
        // Dear ImGui requires physical memory addresses to bind its interactive 
        // widgets to. These variables retain your UI's state between frames.
        // =========================================================================
        
        bool State_IsToggled = false;
        int State_SliderValue = 50;
        float State_FloatValue = 1.0f;
        char State_TextBuffer[256] = "Default Text"; // ImGui uses C-style char arrays by default

        // =========================================================================
        // 3. THE RENDER LOOP (The "Presentation")
        // This is the only function you call inside your main window loop.
        // =========================================================================
        
        void RenderUI() {
            // Create the window panel
            ImGui::Begin("Placeholder_Panel_Name");

            // --- WIDGET 1: A Standard Button ---
            // ImGui::Button returns true ONLY on the exact frame the user clicks it.
            if (ImGui::Button("Trigger_Simple_Action_Label")) {
                if (Event_OnSimpleActionTriggered) { // Check if the engine bound a function
                    Event_OnSimpleActionTriggered(); // Fire the event!
                }
            }

            ImGui::Separator();

            // --- WIDGET 2: A Checkbox (Toggle) ---
            // We pass the memory address (&) of our state boolean. 
            // ImGui automatically flips this boolean when clicked.
            if (ImGui::Checkbox("Toggle_Setting_Label", &State_IsToggled)) {
                // We can detect the exact frame it changed and notify the engine
                if (Event_OnToggleStateChanged) {
                    Event_OnToggleStateChanged(State_IsToggled);
                }
            }

            // --- WIDGET 3: A Slider (Continuous Data) ---
            // Modifies the integer directly. We define the min (0) and max (100) range.
            ImGui::SliderInt("Adjust_Value_Label", &State_SliderValue, 0, 100);

            // --- WIDGET 4: Text Input ---
            // Binds to our char array. The 'sizeof' prevents buffer overflow crashes.
            ImGui::InputText("Text_Input_Label", State_TextBuffer, sizeof(State_TextBuffer));

            // --- WIDGET 5: A Button that uses complex state data ---
            if (ImGui::Button("Submit_Data_Label")) {
                if (Event_OnDataActionTriggered) {
                    // Pass the current slider state out to the engine
                    Event_OnDataActionTriggered(State_SliderValue); 
                }
            }

            ImGui::End();
        }
    };

}