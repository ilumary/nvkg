#ifndef MEM_PROFILER_HPP
#define MEM_PROFILER_HPP

#include <cstddef>
#include <cstdlib>

extern std::size_t alloc_calls_;
extern std::size_t dealloc_calls_;
extern std::size_t used_memory_;

#endif