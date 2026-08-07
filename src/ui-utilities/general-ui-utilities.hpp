#pragma once

#include <string>

// Forward declare
struct ImFont;

// Base class for windows based UI
class UIWindow {
protected:
    std::string window_name;
    
    // For error system
    std::string error_buffer = "";
    bool success = false;
    bool is_busy = false;
public:
    bool is_open;
    explicit UIWindow(const std::string& name, bool startOpen = true);

    virtual ~UIWindow();

    inline const std::string& GetName() const { return window_name; }
    
    inline void status_not_busy() {is_busy = false;};
    inline void status_is_busy() {is_busy = true;};

    // Set error buffer for communication
    inline void set_error_buffer(bool status, std::string msg) {success = status; error_buffer = msg;};
    // Display error buffer as an imgui text message
    void display_error_buffer();
    
    // Pure virtual function: Every window MUST implement its layout logic here
    virtual void Render() = 0;
};


// Global fonts system, to be initialized early in main 
struct SimpleFonts {
    // Font pointers guaranteed to be non-null after init
    ImFont* Fallback = nullptr;// ImGui's built-in default
    ImFont* Small   = nullptr;
    ImFont* Medium  = nullptr; // To be this app's default
    ImFont* Large   = nullptr;

    // Helper push/pop wrappers
    void PushSmall();
    void PushMedium();
    void PushLarge();
    void Pop();
};

// Global instance accessible across ui-windows
extern SimpleFonts G_Fonts;