#pragma once

#include <string>

// Base class for windows based UI
class UIWindow {
protected:
    std::string window_name;
    bool is_open;

public:
    UIWindow(const std::string& name, bool startOpen = true)
        : window_name(name), is_open(startOpen) {}

    virtual ~UIWindow() = default;

    // Pure virtual function: Every window MUST implement its layout logic here
    virtual void Render() = 0;

    // Standard getters/setters for window visibility
    bool IsOpen() const { return is_open; }
    void SetOpen(bool open) { is_open = open; }
    const std::string& GetName() const { return window_name; }
};