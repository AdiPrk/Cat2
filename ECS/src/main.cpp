#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <iostream>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

// Entity!!!
using Entity = std::uint64_t;

constexpr unsigned INDEX_BITS = 32;
constexpr unsigned GENERATION_BITS = 64 - INDEX_BITS;

constexpr Entity INDEX_MASK = ((1ull << INDEX_BITS) - 1); // 0x00000000FFFFFFFF
constexpr Entity GENERATION_MASK = ~INDEX_MASK;           // 0xFFFFFFFF00000000

[[nodiscard]] constexpr std::uint32_t entity_index(Entity e) noexcept {
    return static_cast<std::uint32_t>(e & INDEX_MASK);
}
[[nodiscard]] constexpr std::uint32_t entity_generation(Entity e) noexcept {
    return static_cast<std::uint32_t>((e & GENERATION_MASK) >> INDEX_BITS);
}
[[nodiscard]] constexpr Entity make_entity(std::uint32_t idx, std::uint32_t gen) noexcept {
    return (static_cast<Entity>(gen) << INDEX_BITS) | static_cast<Entity>(idx);
}

// Simple Entity manager: creates/destroys / recycles indices --------------
class EntityManager {
    std::vector<std::uint32_t> generations;
    std::vector<std::uint32_t> free_list; // indices available for reuse
public:
    Entity create() {
        std::uint32_t idx;
        if (!free_list.empty()) {
            idx = free_list.back();
            free_list.pop_back();
        }
        else {
            idx = static_cast<std::uint32_t>(generations.size());
            generations.push_back(0);
        }
        return make_entity(idx, generations[idx]);
    }

    void destroy(Entity e) {
        auto idx = entity_index(e);
        if (idx >= generations.size()) return;
        // bump generation so stale Entity handles become invalid
        ++generations[idx];
        free_list.push_back(idx);
    }

    bool alive(Entity e) const {
        auto idx = entity_index(e);
        if (idx >= generations.size()) return false;
        return entity_generation(e) == generations[idx];
    }

    std::uint32_t generation(std::uint32_t idx) const {
        return idx < generations.size() ? generations[idx] : 0;
    }
};

// Sparse Set ----------------------------------------------------------------
template <typename T>
class SparseSet {
    static constexpr size_t npos = static_cast<size_t>(-1);

    std::vector<Entity> dense;     // store actual entity ids
    std::vector<size_t> sparse;    // map from entity index -> position in dense
    std::vector<T> components;     // component storage parallel to dense

    void ensure_sparse_size(std::uint32_t idx) {
        if (idx >= sparse.size()) {
            sparse.resize(idx + 1, npos);
        }
    }

public:
    SparseSet() = default;

    bool contains(Entity e) const noexcept {
        auto idx = entity_index(e);
        if (idx >= sparse.size()) return false;
        size_t pos = sparse[idx];
        return pos != npos && pos < dense.size() && dense[pos] == e;
    }

    // insert a component for entity (if exists, overwrite)
    T& insert(Entity e, T component) {
        auto idx = entity_index(e);
        ensure_sparse_size(idx);

        if (contains(e)) {
            size_t pos = sparse[idx];
            components[pos] = std::move(component);
            return components[pos];
        }

        size_t pos = dense.size();
        sparse[idx] = pos;
        dense.push_back(e);
        components.push_back(std::move(component));
        return components.back();
    }

    // remove component for entity (if exists)
    void remove(Entity e) noexcept {
        if (!contains(e)) return;
        auto idx = entity_index(e);
        size_t pos = sparse[idx];
        size_t last = dense.size() - 1;

        if (pos != last) {
            // move last element into pos
            dense[pos] = dense[last];
            components[pos] = std::move(components[last]);
            // update sparse for the moved entity
            sparse[entity_index(dense[pos])] = pos;
        }
        // pop the last
        dense.pop_back();
        components.pop_back();
        sparse[idx] = npos;
    }

    // get component by entity (throws if not present)
    T& get(Entity e) {
        if (!contains(e)) throw std::runtime_error("Entity has no component");
        return components[sparse[entity_index(e)]];
    }
    const T& get(Entity e) const {
        if (!contains(e)) throw std::runtime_error("Entity has no component");
        return components[sparse[entity_index(e)]];
    }

    size_t size() const noexcept { return dense.size(); }

    // iterate (entity, component) pairs
    template<typename Func>
    void each(Func f) {
        for (size_t i = 0; i < dense.size(); ++i) {
            f(dense[i], components[i]);
        }
    }

    void clear() {
        dense.clear();
        components.clear();
        sparse.assign(sparse.size(), npos);
    }
};

template <typename... Ts>
struct type_list { static constexpr size_t size = sizeof...(Ts); };

template <typename T, typename... Ts>
constexpr std::size_t type_index_v_helper() {
    std::size_t i = 0;
    ((i += !std::is_same_v<T, Ts>), ...);
    return i;
}

template <typename T, typename... Ts>
constexpr std::size_t type_index_v = type_index_v_helper<T, Ts...>();

template <typename T, typename TL>
constexpr std::size_t type_index_of(const TL&)
{
    return type_index_v<T, TL>;
}

// Example usage -------------------------------------------------------------
struct Position { float x, y; };
struct Velocity { float vx, vy; };

int main() {
    EntityManager em;

    // create two entities
    Entity e1 = em.create();
    Entity e2 = em.create();

    // simple sparse set for Position
    SparseSet<Position> positions;
    positions.insert(e1, Position{ 1.0f, 2.0f });
    positions.insert(e2, Position{ 3.0f, 4.0f });

    // iterate
    positions.each([&](Entity e, Position& p) {
        std::cout << "Entity idx=" << entity_index(e)
            << " gen=" << entity_generation(e)
            << " pos=(" << p.x << "," << p.y << ")\n";
        });

    // remove e1
    positions.remove(e1);
    std::cout << "after remove, size = " << positions.size() << "\n";

    type_list<int, float, double> types;
    std::size_t ind = type_index_v_helper<float, type_list<int, float, double>>();
    ind = type_index_of<float>(types);
    std::cout << ind << std::endl;
}

/*


// Generation (bits 63-32) | Index (bits 31-0)
constexpr unsigned INDEX_BITS = 32;
constexpr unsigned GENERATION_BITS = 64 - INDEX_BITS;

constexpr Entity INDEX_MASK = ((1ull << INDEX_BITS) - 1); // 0x00000000FFFFFFFF
constexpr Entity GENERATION_MASK = ~INDEX_MASK;           // 0xFFFFFFFF00000000

inline constexpr std::uint32_t entity_index(Entity e)
{
    return static_cast<std::uint32_t>(e & INDEX_MASK);
}

inline constexpr std::uint32_t entity_generation(Entity e)
{
    return static_cast<std::uint32_t>((e & GENERATION_MASK) >> INDEX_BITS);
}

inline constexpr Entity make_entity(std::uint32_t index, std::uint32_t generation) 
{
    return (static_cast<Entity>(generation) << INDEX_BITS) | static_cast<Entity>(index);
}

// -------------------- Typelist + compile-time type index --------------------

template <typename... Ts>
struct type_list { static constexpr size_t size = sizeof...(Ts); };

template <typename T, typename... Ts>
constexpr std::size_t type_index_v_helper() {
    std::size_t i = 0;
    ((i += !std::is_same_v<T, Ts>), ...);
    return i;
}

template <typename T, typename... Ts>
constexpr std::size_t type_index_v = type_index_v_helper<T, Ts...>();

template <typename T, typename TL>
constexpr std::size_t type_index_of(const TL&)
{
    return type_index_v<T, TL>;
}


class SparseSet
{
public:

private:
    
};



int main()
{
    type_list<int, float, double> types;
    std::size_t ind = type_index_v_helper<float, type_list<int, float, double>>();
    ind = type_index_of<float>(types);
    std::cout << ind << std::endl;
}
*/