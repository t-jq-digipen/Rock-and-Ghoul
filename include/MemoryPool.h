/* ========================================================
 * @file        MemoryPool.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        24 JAN 2026
 *
 * @brief
 * Custom memory pool allocator for game objects and components.
 * Pre-allocates large memory chunks at load time and hands out
 * pieces during gameplay with no runtime heap allocations.
 *
 * Features:
 * - Fixed-size block allocation for specific types
 * - Free list management for deallocated blocks
 * - Placement new support for component construction
 * - Cache-friendly contiguous memory layout
 * - No runtime OS allocations after initialization
 *
 * Copyright © 2026 DigiPen, All rights reserved.
 * ========================================================
 */

#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <new>      // For placement new
#include <utility>  // For std::forward

 // ========================================================
 // Memory Block - represents one unit in the free list
 // ========================================================
struct MemoryBlock {
    MemoryBlock* next;  // Points to next free block
};

// ========================================================
// MemoryPool - manages a pool of fixed-size memory blocks
// ========================================================
template<typename T, size_t PoolSize>
class MemoryPool {
public:
    MemoryPool();
    ~MemoryPool();

    // Allocate one object from the pool
    T* Allocate();

    // Deallocate an object (returns it to free list)
    void Deallocate(T* ptr);

    // Construct object in-place (uses placement new)
    template<typename... Args>
    T* Construct(Args&&... args);

    // Destroy object (calls destructor but doesn't deallocate)
    void Destroy(T* ptr);

    // Get stats
    size_t GetCapacity() const { return PoolSize; }
    size_t GetUsedCount() const { return usedCount; }
    size_t GetFreeCount() const { return PoolSize - usedCount; }
    bool IsFull() const { return usedCount >= PoolSize; }
    bool IsEmpty() const { return usedCount == 0; }

    // Check if pointer belongs to this pool
    bool OwnsPointer(T* ptr) const;

private:
    // Pre-allocated memory buffer (aligned properly for T)
    alignas(T) uint8_t memory[sizeof(T) * PoolSize];

    // Free list head
    MemoryBlock* freeList;

    // Usage tracking
    size_t usedCount;

    // Initialize the free list
    void InitializeFreeList();
};

// ========================================================
// Template Implementation
// ========================================================

template<typename T, size_t PoolSize>
MemoryPool<T, PoolSize>::MemoryPool()
    : freeList(nullptr)
    , usedCount(0) {
    InitializeFreeList();
}

template<typename T, size_t PoolSize>
MemoryPool<T, PoolSize>::~MemoryPool() {
    // Note: This doesn't call destructors automatically
    // User must ensure all objects are properly destroyed before pool destruction
}

template<typename T, size_t PoolSize>
void MemoryPool<T, PoolSize>::InitializeFreeList() {
    // Chain all blocks together in the free list
    freeList = reinterpret_cast<MemoryBlock*>(memory);

    MemoryBlock* current = freeList;
    uint8_t* blockPtr = memory;

    for (size_t i = 0; i < PoolSize - 1; ++i) {
        blockPtr += sizeof(T);
        current->next = reinterpret_cast<MemoryBlock*>(blockPtr);
        current = current->next;
    }

    current->next = nullptr;  // Last block points to null
}

template<typename T, size_t PoolSize>
T* MemoryPool<T, PoolSize>::Allocate() {
    if (freeList == nullptr) {
        // Pool exhausted - return nullptr instead of asserting
        // This allows graceful handling of exhaustion
        return nullptr;
    }

    // Pop from free list
    MemoryBlock* block = freeList;
    freeList = freeList->next;

    ++usedCount;

    return reinterpret_cast<T*>(block);
}

template<typename T, size_t PoolSize>
void MemoryPool<T, PoolSize>::Deallocate(T* ptr) {
    if (ptr == nullptr) return;

    assert(OwnsPointer(ptr) && "Attempting to deallocate pointer not from this pool!");

    // Push back to free list
    MemoryBlock* block = reinterpret_cast<MemoryBlock*>(ptr);
    block->next = freeList;
    freeList = block;

    --usedCount;
}

template<typename T, size_t PoolSize>
template<typename... Args>
T* MemoryPool<T, PoolSize>::Construct(Args&&... args) {
    T* ptr = Allocate();
    if (ptr != nullptr) {
        // Placement new - construct object in allocated memory
        new (ptr) T(std::forward<Args>(args)...);
    }
    return ptr;
}

template<typename T, size_t PoolSize>
void MemoryPool<T, PoolSize>::Destroy(T* ptr) {
    if (ptr != nullptr) {
        assert(OwnsPointer(ptr) && "Attempting to destroy pointer not from this pool!");

        // Call destructor explicitly
        ptr->~T();

        // Return memory to pool
        Deallocate(ptr);
    }
}

template<typename T, size_t PoolSize>
bool MemoryPool<T, PoolSize>::OwnsPointer(T* ptr) const {
    uint8_t* bytePtr = reinterpret_cast<uint8_t*>(ptr);
    uint8_t* poolStart = const_cast<uint8_t*>(memory);
    uint8_t* poolEnd = poolStart + (sizeof(T) * PoolSize);

    return (bytePtr >= poolStart && bytePtr < poolEnd);
}

#endif // MEMORY_POOL_H