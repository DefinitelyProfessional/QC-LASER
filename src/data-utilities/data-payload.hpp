#pragma once

#include <optional>
#include <string>

struct StatusPayload {
    bool success;
    std::string msg;
};

template<typename T> struct DataPayload {
    bool success;
    std::string msg;
    std::optional<T> data;
};