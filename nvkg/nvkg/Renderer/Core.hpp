#pragma once

#include <nvkg/Renderer/Utils/Hash.hpp>

#include <volk/volk.h>
#include <iostream>
#include <cstdint>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#define EXIT_APP abort();

#define PRINT_ASSERT_FAILURE(expr, file, line, message) \
    std::cout << "NVKG ASSERTION FAILURE: " << #expr << " in file: " << file << " on line: "  << line << std::endl; \
    std::cout  << "                        Message: " << message << std::endl; 

// Custom assert macro
#define NVKG_ASSERT(expr, message) \
    if (expr) { } \
    else { \
        PRINT_ASSERT_FAILURE(#expr, __FILE__, __LINE__, message); \
        EXIT_APP \
    } \

#define OUT

#define INTERN_STR(str) Utils::WSID(str)

namespace nvkg {
    struct CameraData {
        glm::mat4 projectionMatrix;
        glm::mat4 viewMatrix;
    };
}