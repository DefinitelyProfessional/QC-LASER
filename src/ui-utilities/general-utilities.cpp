#include "ui-utilities/general-utilities.hpp"

// Constructor and Destructor definition to reduce vtable lookup
UIWindow::UIWindow(const std::string& name, bool startOpen) : window_name(name), is_open(startOpen) {}
UIWindow::~UIWindow() = default;