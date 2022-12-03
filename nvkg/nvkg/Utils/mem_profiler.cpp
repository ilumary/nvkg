#include <nvkg/Utils/mem_profiler.hpp>

std::size_t alloc_calls_{0};
std::size_t dealloc_calls_{0};
std::size_t used_memory_{0};

void* operator new(std::size_t size) {
    alloc_calls_ += 1;
    used_memory_ += size;
    return std::malloc(size);
}

void operator delete(void* memory, std::size_t size) {
    dealloc_calls_ += 1;
    used_memory_ -= size;
    std::free(memory);
}

void* operator new[](std::size_t size) {
    alloc_calls_ += 1;
    used_memory_ += size;
    return std::malloc(size);
}

void operator delete[](void* memory, std::size_t size) {
    dealloc_calls_ += 1;
    used_memory_ -= size;
    std::free(memory);
}