#ifndef HASH_HPP
#define HASH_HPP

#include <ranges>
#include <iostream>
#include <string>
#include <vector>

namespace ecs {

    constexpr uint64_t fast_hash(std::string_view i) {
        size_t hash = sizeof(size_t) == 8 ? 0xcbf29ce484222325 : 0x811c9dc5;
        const size_t prime = sizeof(size_t) == 8 ? 0x00000100000001b3 : 0x01000193;

        for(char c : i) {
            hash ^= static_cast<size_t>(c);
            hash *= prime;
        }

        return hash;
    }

    template <typename ... T>
    consteval std::string_view TypeName() noexcept {
        using namespace std::literals;
        constexpr std::string_view func{
            __PRETTY_FUNCTION__
        };
        auto const first{ func.find(__func__) + std::size(__func__) };
        return func.substr(first, func.find(">("sv, first) - first);
    }

    template <typename ... T>
    consteval uint64_t TypeHash() noexcept {
        return fast_hash(TypeName<T...>());
    }
}

#endif
