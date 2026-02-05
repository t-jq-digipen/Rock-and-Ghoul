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
 * Copyright ? 2026 DigiPen, All rights reserved.
 * ========================================================
 */


#ifndef MEMORYPOOL_H
#define MEMORYPOOL_H

#include <iostream>
#include <cstdint>
#include <cstring>

/**
 * @brief Memory block structure for free list management
 *
 * Uses intrusive linked list - stores next pointer in unused memory
 */
struct MemoryBlock {
    MemoryBlock* next;
};

/**
 * @brief Single memory page containing multiple fixed-size objects
 *
 * @tparam T Object type to store in this page
 * @tparam ObjectsPerPage Number of objects this page can hold
 */
template<typename T, size_t ObjectsPerPage>
struct MemoryPage {
    alignas(T) uint8_t memory[sizeof(T) * ObjectsPerPage];  // Object storage
    MemoryPage* next;  // Link to next page in page list

    MemoryPage() : next(nullptr) {}
};

/**
 * @brief Dynamic memory pool with automatic page allocation
 *
 * Pre-allocates first page at construction, then dynamically allocates
 * additional pages when pool is exhausted (up to maxPages limit).
 * Uses free-list for O(1) allocation/deallocation.
 *
 * @tparam T Type of objects to allocate
 * @tparam ObjectsPerPage Number of objects per page
 */
template<typename T, size_t ObjectsPerPage>
class DynamicMemoryPool {
private:
    using Page = MemoryPage<T, ObjectsPerPage>;

    Page* pageList_;         // Linked list of all allocated pages
    MemoryBlock* freeList_;  // Linked list of free memory blocks
    size_t usedCount_;       // Number of objects currently in use
    size_t totalCapacity_;   // Total objects across all pages
    size_t maxPages_;        // Maximum pages allowed (0 = unlimited)
    size_t currentPages_;    // Current number of allocated pages

public:
    /**
     * @brief Construct dynamic memory pool
     *
     * @param maxPages Maximum pages to allocate (0 = unlimited)
     */
    explicit DynamicMemoryPool(size_t maxPages = 0)
        : pageList_(nullptr)
        , freeList_(nullptr)
        , usedCount_(0)
        , totalCapacity_(0)
        , maxPages_(maxPages)
        , currentPages_(0)
    {
        // Allocate first page at construction (LOAD TIME)
        AllocateNewPage();
    }

    /**
     * @brief Destructor - frees all allocated pages
     */
    ~DynamicMemoryPool() {
        while (pageList_ != nullptr) {
            Page* next = pageList_->next;
            delete pageList_;
            pageList_ = next;
        }
    }

    /**
     * @brief Allocate object from pool (RUNTIME BOOKING)
     *
     * If pool is exhausted, attempts to allocate new page.
     * Returns nullptr if allocation fails or max pages reached.
     *
     * @return Pointer to allocated object, or nullptr on failure
     */
    T* Allocate() {
        // Pool exhausted - try to allocate new page
        if (freeList_ == nullptr) {
            std::cout << "[DynamicPool] Pool exhausted! Attempting dynamic page allocation...\n";
            AllocateNewPage();

            // Check if allocation succeeded
            if (freeList_ == nullptr) {
                std::cerr << "[DynamicPool] Cannot allocate: Max pages reached or allocation failed\n";
                return nullptr;
            }
        }

        // Pop from free list (O(1))
        MemoryBlock* block = freeList_;
        freeList_ = freeList_->next;
        ++usedCount_;

        return reinterpret_cast<T*>(block);
    }

    /**
     * @brief Return object to pool
     *
     * @param ptr Pointer to object to deallocate
     */
    void Deallocate(T* ptr) {
        if (ptr == nullptr) return;

        // Push to free list (O(1))
        MemoryBlock* block = reinterpret_cast<MemoryBlock*>(ptr);
        block->next = freeList_;
        freeList_ = block;
        --usedCount_;
    }

    /**
     * @brief Construct object in allocated memory using placement new
     *
     * @param args Constructor arguments
     * @return Pointer to constructed object
     */
    template<typename... Args>
    T* Construct(Args&&... args) {
        T* ptr = Allocate();
        if (ptr != nullptr) {
            new (ptr) T(std::forward<Args>(args)...);  // Placement new
        }
        return ptr;
    }

    /**
     * @brief Destroy object and return memory to pool
     *
     * @param ptr Pointer to object to destroy
     */
    void Destroy(T* ptr) {
        if (ptr != nullptr) {
            ptr->~T();  // Explicit destructor call
            Deallocate(ptr);
        }
    }

    /**
     * @brief Get number of objects currently in use
     */
    size_t GetUsedCount() const { return usedCount_; }

    /**
     * @brief Get total capacity across all pages
     */
    size_t GetTotalCapacity() const { return totalCapacity_; }

    /**
     * @brief Get number of free objects available
     */
    size_t GetFreeCount() const { return totalCapacity_ - usedCount_; }

    /**
     * @brief Get number of allocated pages
     */
    size_t GetPageCount() const { return currentPages_; }

    /**
     * @brief Print statistics about this pool
     */
    void PrintStats(const char* poolName) const {
        std::cout << poolName << ": "
            << usedCount_ << "/" << totalCapacity_ << " used"
            << " (" << currentPages_ << " pages)\n";
    }

private:
    /**
     * @brief Allocate new page and add objects to free list
     *
     * This is called at construction (LOAD TIME) and when pool
     * is exhausted at runtime (DYNAMIC EXTENSION).
     */
    void AllocateNewPage() {
        // Check page limit
        if (maxPages_ > 0 && currentPages_ >= maxPages_) {
            std::cerr << "[DynamicPool] Maximum pages (" << maxPages_
                << ") reached. Cannot allocate more.\n";
            return;
        }

        std::cout << "[DynamicPool] Allocating new page "
            << (currentPages_ + 1);
        if (maxPages_ > 0) {
            std::cout << "/" << maxPages_;
        }
        std::cout << " (runtime dynamic allocation)...\n";

        // Allocate new page using operator new (RUNTIME OS ALLOCATION)
        Page* newPage = nullptr;
        try {
            newPage = new Page();
        }
        catch (const std::bad_alloc&) {
            std::cerr << "[DynamicPool] Failed to allocate page: Out of system memory\n";
            return;
        }

        // Link page to page list
        newPage->next = pageList_;
        pageList_ = newPage;

        // Add all objects from this page to free list
        uint8_t* pageMemory = newPage->memory;
        for (size_t i = 0; i < ObjectsPerPage; ++i) {
            MemoryBlock* block = reinterpret_cast<MemoryBlock*>(
                pageMemory + (i * sizeof(T))
                );

            // Push to free list
            block->next = freeList_;
            freeList_ = block;
        }

        // Update statistics
        ++currentPages_;
        totalCapacity_ += ObjectsPerPage;

        std::cout << "[DynamicPool] Page allocated successfully! "
            << "Total capacity: " << totalCapacity_ << " objects across "
            << currentPages_ << " pages.\n";
    }

    // Prevent copying
    DynamicMemoryPool(const DynamicMemoryPool&) = delete;
    DynamicMemoryPool& operator=(const DynamicMemoryPool&) = delete;
};

#endif // MEMORYPOOL_H