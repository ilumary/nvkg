#pragma once

#include <algorithm>
#include <bitset>
#include <cstdint>
#include <functional>
#include <numeric>
#include <stdexcept>

#include <nvkg/ecs/detail/dynamic_bitset.hpp>
#include <nvkg/ecs/detail/hash_map.hpp>
#include <nvkg/ecs/detail/type_traits.hpp>
#include <nvkg/ecs/type_meta.hpp>

namespace ecs {

namespace detail {

template<typename = void, typename _id_type = std::uint64_t>
class type_registry {
public:
    using id_type = _id_type;

#ifndef CO_ECS_CLIENT
    CO_ECS_EXPORT static id_type id(std::string_view type_string) {
        auto [iter, inserted] = get_id_map().emplace(type_string, get_next_id());
        if (inserted) {
            get_next_id()++;
        }
        auto type_id = iter->second;
        return type_id;
    }

    CO_ECS_EXPORT static hash_map<std::string_view, id_type>& get_id_map() {
        static hash_map<std::string_view, id_type> id_map{};
        return id_map;
    }

    CO_ECS_EXPORT static id_type& get_next_id() {
        static id_type next_id{};
        return next_id;
    }
#else
    CO_ECS_IMPORT static hash_map<std::string_view, id_type>& get_id_map();
    CO_ECS_IMPORT static id_type& get_next_id();
    CO_ECS_IMPORT static id_type id(std::string_view type_string);
#endif
};

/// @brief Generate unique sequential IDs for types
///
/// @tparam Base type
/// @tparam _id_type Type for id
template<typename Base = void, typename _id_type = std::uint64_t>
struct type_id {
    using id_type = _id_type;

    template<typename T>
    inline static const id_type value = type_registry<Base, id_type>::id(type_name<T>());
};

} // namespace detail

/// @brief Type for component ID
using component_id_t = std::uint32_t;

/// @brief Invalid component ID
constexpr auto invalid_component_id = std::numeric_limits<component_id_t>::max();

/// @brief Type for family used to generated component IDs.
using component_id = detail::type_id<struct _component_family_t, component_id_t>;

/// @brief Component concept. The component must be a struct/class that can be move constructed and move
/// assignable
///
/// @tparam T Component type
template<typename T>
concept component =
    std::is_class_v<T> && std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>;

/// @brief Component reference concept. It should be a reference or const reference to C, where C satisfies component
/// concept
///
/// @tparam T Component reference type
template<typename T>
concept component_reference = std::is_reference_v<T> && component<std::remove_cvref_t<T>>;

/// @brief Decay component; converts component_reference to component by removing cv-qualifiers and reference.
///
/// @tparam T Type
template<component_reference T>
using decay_component_t = std::decay_t<T>;

/// @brief Struct to determine const-ness of component reference type
///
/// @tparam T component reference type
template<component_reference T>
struct const_component_reference {
    constexpr static bool value = std::is_const_v<std::remove_reference_t<T>>;
};

/// @brief Returns true for const component references
///
/// @tparam T component_reference type
template<component_reference T>
constexpr bool const_component_reference_v = const_component_reference<T>::value;

/// @brief Struct to determine mutability of component reference type
///
/// @tparam T component reference type
template<component_reference T>
struct mutable_component_reference {
    constexpr static bool value = !std::is_const_v<std::remove_reference_t<T>>;
};

/// @brief Returns true for non-const component references
///
/// @tparam T component_reference type
template<component_reference T>
constexpr bool mutable_component_reference_v = mutable_component_reference<T>::value;

/// @brief Struct to determine whether all component references are const
///
/// @tparam Args Component references
template<component_reference... Args>
struct const_component_references {
    constexpr static bool value = std::conjunction_v<const_component_reference<Args>...>;
};

/// @brief Returns true when all Args are const references
///
/// @tparam Args Component references
template<component_reference... Args>
constexpr bool const_component_references_v = const_component_references<Args...>::value;

/// @brief Component metadata. Stores an ID, size, alignment, destructor, etc.
struct component_meta {
public:
    /// @brief Constructs component_meta for type T
    ///
    /// @tparam T Component type
    /// @return component_meta Component metadata
    template<component T>
    static component_meta of() noexcept {
        return component_meta{
            component_id::value<T>,
            type_meta::of<T>(),
        };
    }

    /// @brief Spaceship operator
    ///
    /// @param rhs Right hand side
    /// @return auto Result of comparison
    constexpr auto operator<=>(const component_meta& rhs) const noexcept {
        return id <=> rhs.id;
    }

    /// @brief Equality operator
    ///
    /// @param rhs Right hand side
    /// @return true If equal
    /// @return false If not equal
    constexpr bool operator==(const component_meta& rhs) const noexcept {
        return id == rhs.id;
    }

    component_id_t id;
    const type_meta* type;
};

/// @brief Component set holds a set of component IDs
class component_set {
public:
    using storage_type = detail::dynamic_bitset<>;

    /// @brief Construct component set from given component types
    ///
    /// @tparam Args Components type parameter pack
    /// @return component_set Component set
    template<component... Args>
    static component_set create() {
        component_set s;
        (..., s.insert<Args>());
        return s;
    }

    /// @brief Insert component of type T
    ///
    /// @tparam T Component type
    template<component T>
    void insert() {
        insert(component_id::value<T>);
    }

    /// @brief Erase component of type T
    ///
    /// @tparam T Component type
    template<component T>
    void erase() {
        erase(component_id::value<T>);
    }

    /// @brief Check if component of type T is present in the set
    ///
    /// @tparam T Component type
    /// @return true When component type T is present
    /// @return false When component type T is not present
    template<component T>
    [[nodiscard]] bool contains() const {
        return contains(component_id::value<T>);
    }

    /// @brief Inserts component into the set
    ///
    /// @param id Component ID
    void insert(component_id_t id) {
        _bitset.set(id);
    }

    /// @brief Erases component from the set
    ///
    /// @param id Component ID
    void erase(component_id_t id) {
        _bitset.set(id, false);
    }

    /// @brief Check if component is present in the set
    ///
    /// @param id Component ID
    /// @return true When component ID is present
    /// @return false When component ID is not present
    [[nodiscard]] bool contains(component_id_t id) const {
        return _bitset.test(id);
    }

    void clear() noexcept {
        _bitset.clear();
    }

    /// @brief Equality operator
    ///
    /// @param rhs Right hand side
    /// @return true If equal
    /// @return false If not equal
    bool operator==(const component_set& rhs) const noexcept {
        return _bitset == rhs._bitset;
    }

private:
    friend class component_set_hasher;
    storage_type _bitset{};
};

/// @brief Component set hasher
class component_set_hasher {
public:
    /// @brief Hash component set
    ///
    /// @param set Component set
    /// @return std::size_t Hash value
    std::size_t operator()(const component_set& set) const {
        return std::hash<typename component_set::storage_type>()(set._bitset);
    }
};

/// @brief Component set holds a set of components metadata
class component_meta_set {
public:
    using size_type = std::size_t;
    using storage_type = std::vector<component_meta>;
    using value_type = typename storage_type::value_type;
    using const_iterator = typename storage_type::const_iterator;

    /// @brief Construct component set from given component types
    ///
    /// @tparam Args Components type parameter pack
    /// @return component_meta_set Component set
    template<component... Args>
    static component_meta_set create() {
        component_meta_set s;
        s._components_meta.reserve(sizeof...(Args));
        (..., s.insert<Args>());
        return s;
    }

    /// @brief Insert component of type T
    ///
    /// @tparam T Component type
    template<component T>
    void insert() {
        insert(component_meta::of<T>());
    }

    /// @brief Erase component of type T
    ///
    /// @tparam T Component type
    template<component T>
    void erase() {
        erase(component_id::value<T>);
    }

    /// @brief Check if component of type T is present in the set
    ///
    /// @tparam T Component type
    /// @return true When component type T is present
    /// @return false When component type T is not present
    template<component T>
    [[nodiscard]] bool contains() const {
        return contains(component_id::value<T>);
    }

    /// @brief Inserts component into the set
    ///
    /// @param meta Component meta
    void insert(const value_type& meta) {
        if (contains(meta.id)) {
            return;
        }
        _component_set.insert(meta.id);
        _components_meta.emplace_back(meta);
    }

    /// @brief Erases component from the set
    ///
    /// @param id Component ID
    void erase(component_id_t id) {
        if (!contains(id)) {
            return;
        }
        _component_set.erase(id);
        std::erase_if(_components_meta, [id](const auto& meta) { return meta.id == id; });
    }

    /// @brief Check if component is present in the set
    ///
    /// @param id Component ID
    /// @return true When component ID is present
    /// @return false When component ID is not present
    [[nodiscard]] bool contains(component_id_t id) const {
        return _component_set.contains(id);
    }

    /// @brief Returns how many components in the set
    ///
    /// @return size_type Number of components in the set
    [[nodiscard]] size_type size() const noexcept {
        return _components_meta.size();
    }

    /// @brief Return const iterator to beginning of the set
    ///
    /// @return const_iterator Iterator
    [[nodiscard]] const_iterator begin() const noexcept {
        return _components_meta.begin();
    }

    /// @brief Return const iterator to the end of the set
    ///
    /// @return const_iterator Iterator
    [[nodiscard]] const_iterator end() const noexcept {
        return _components_meta.end();
    }

    /// @brief Return const iterator to beginning of the set
    ///
    /// @return const_iterator Iterator
    [[nodiscard]] const_iterator cbegin() const noexcept {
        return begin();
    }

    /// @brief Return const iterator to the end of the set
    ///
    /// @return const_iterator Iterator
    [[nodiscard]] const_iterator cend() const noexcept {
        return end();
    }

    /// @brief Equality operator
    ///
    /// @param rhs Right hand side
    /// @return true If equal
    /// @return false If not equal
    bool operator==(const component_meta_set& rhs) const noexcept {
        return _component_set == rhs._component_set;
    }

    /// @brief Return a bitset of components
    ///
    /// @return const component_set& Component bitset
    [[nodiscard]] const component_set& ids() const noexcept {
        return _component_set;
    }

private:
    component_set _component_set;
    storage_type _components_meta;
};

} // namespace ecs
