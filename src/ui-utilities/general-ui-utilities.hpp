#pragma once

#include <string>

// Base class for windows based UI
class UIWindow {
protected:
    std::string window_name;
    
public:
    bool is_open;
    explicit UIWindow(const std::string& name, bool startOpen = true);

    virtual ~UIWindow();

    inline const std::string& GetName() const { return window_name; }
    
    // Pure virtual function: Every window MUST implement its layout logic here
    virtual void Render() = 0;
};