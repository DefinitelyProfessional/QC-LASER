#pragma once

#include <optional>
#include <string>

struct StatusPayload {
    std::string msg;
    bool success;
};

template<typename T> struct DataPayload {
    std::optional<T> data;
    std::string msg;
    bool success;
};