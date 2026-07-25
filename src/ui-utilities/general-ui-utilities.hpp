#pragma once

#include <string>

// Base class for windows based UI
class UIWindow {
protected:
    std::string window_name;
    bool is_open;

public:
    explicit UIWindow(const std::string& name, bool startOpen = true);

    virtual ~UIWindow();

    // Pure virtual function: Every window MUST implement its layout logic here
    virtual void Render() = 0;

    // Standard getters/setters for window visibility
    inline bool IsOpen() const { return is_open; }
    inline void SetOpen(bool open) { is_open = open; }
    inline const std::string& GetName() const { return window_name; }
};