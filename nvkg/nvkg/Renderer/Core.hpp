#ifndef NVKG_CORE_HPP
#define NVKG_CORE_HPP

#include <nvkg/Renderer/Utils/Hash.hpp>
#include <nvkg/Utils/mem_profiler.hpp>
#include <nvkg/Utils/logger.hpp>
#include <nvkg/ecs/ecs.hpp>
#include <nvkg/Utils/threadpool.hpp> //TODO replace with own 

#include <volk/volk.h>
#include <iostream>
#include <cstdint>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#define EXIT_APP abort();

#define PRINT_ASSERT_FAILURE(expr, file, line, message) \
    logger::debug(logger::Level::Fatal) << "NVKG ASSERTION FAILURE: " << #expr << " in file: " << file << " on line: "  << line; \
    logger::debug(logger::Level::Fatal)  << "                        Message: " << message;

// Custom assert macro
#define NVKG_ASSERT(expr, message) \
    if (expr) { } \
    else { \
        PRINT_ASSERT_FAILURE(#expr, __FILE__, __LINE__, message); \
        EXIT_APP \
    } \

#define OUT

#define INTERN_STR(str) Utils::WSID(str)

#endif
