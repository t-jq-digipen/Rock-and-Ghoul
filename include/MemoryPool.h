/* ========================================================
 * @file        MemoryPool.h
 * @author      Wong Shi Yu Regine | w.shiyuregine@digipen.edu
 * @date        5 FEB 2026
 *
 * @brief
 * Custom memory pool allocator for game objects.
 * Pre-allocates large memory chunks at load time and hands out
 * pieces during gameplay with no runtime heap allocations.
 *
 * - Fixed-size block allocation for diff types
 * - Free list management for deallocated blocks
 *
 * Copyright © 2026 DigiPen, All rights reserved.
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
 * Uses linked list stores next pointer in unused memory
 */
struct MemoryBlock {
    MemoryBlock* next;
};

/**
 * @brief Single memory page containing multiple fixed-size objects
 *
 * Object type to store in this page
 * ObjectsPerPage Number of objects this page can hold
 */
template<typename T, size_t ObjectsPerPage>
struct MemoryPage {
    alignas(T) uint8_t memory[sizeof(T) * ObjectsPerPage];  // raw mem block https://global.php.cn/faq/1796879898.html
    MemoryPage* next;  // Link to next page in page list

    MemoryPage() : next(nullptr) {}
};

/**
 * @brief Dynamic memory pool with page allocation
 *
 * Pre-allocates first page at construction, then dynamically allocates
 * additional pages when pool is exhausted.
 * free-list for O(1) allocation n deallocation.
 *
 *  Type of objects to allocate
 *  ObjectsPerPage Number of objects per page
 */
template<typename T, size_t ObjectsPerPage>
class DynamicMemoryPool {
private:
    using Page = MemoryPage<T, ObjectsPerPage>;

    Page* pageList_;         // Linked list of all allocated pages
    MemoryBlock* freeList_;  // Linked list of free memory blocks
    size_t usedCount_;       // Num of objects in use
    size_t totalCapacity_;   // Total objects
    size_t maxPages_;
    size_t currentPages_;    // Number of allocated pages

public:
    
    DynamicMemoryPool(size_t maxPages = 0) : pageList_(nullptr), freeList_(nullptr), usedCount_(0), totalCapacity_(0), maxPages_(maxPages), currentPages_(0)
    {
        // Allocate first page at load
        AllocateNewPage();
    }

    /**
     * @brief frees all allocated pages
     */
    ~DynamicMemoryPool() {
        while (pageList_ != nullptr) {
            Page* next = pageList_->next;
            delete pageList_;
            pageList_ = next;
        }
    }

    /**
     * @brief Allocate object from pool
     *
     * If pool is exhausted, attempts to allocate new page.
     * Returns nullptr if allocation fails or max pages reached.
     *
     * @return Pointer to allocated object, or nullptr on failure
     */
    T* Allocate() {
        // Pool exhausted try to allocate new page
        if (freeList_ == nullptr) {
            //std::cout << "[DynamicPool] Pool exhausted\n";
            AllocateNewPage();

            
            if (freeList_ == nullptr) {
                
                return nullptr;
            }
        }

        // Pop free list
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

        // Push to free list
        MemoryBlock* block = reinterpret_cast<MemoryBlock*>(ptr);
        block->next = freeList_;
        freeList_ = block;
        --usedCount_;
    }


    template<typename... Args>
    T* Construct(Args&&... args) {
        T* ptr = Allocate();
        if (ptr != nullptr) {
            //yay useful from hlp https://cpp-primer.pages.dev/book/177-19.1._controlling_memory_allocation
            //need this for ref learning https://www.geeksforgeeks.org/cpp/placement-new-operator-cpp/
            new (ptr) T(std::forward<Args>(args)...);
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
            ptr->~T();
            Deallocate(ptr);
        }
    }

    /**
     * @brief Get number of objects in use
     */
    size_t GetUsedCount() const { return usedCount_; }

    /**
     * @brief Get total capacity across all pages
     */
    size_t GetTotalCapacity() const { return totalCapacity_; }

    /**
     * @brief Get number of free objects
     */
    size_t GetFreeCount() const { return totalCapacity_ - usedCount_; }

    /**
     * @brief Get number of allocated pages
     */
    size_t GetPageCount() const { return currentPages_; }

    /**
     * @brief Print
     */
    void PrintStats(const char* poolName) const {
        std::cout << poolName << ": " << usedCount_ << "/" << totalCapacity_ << " used" << " (" << currentPages_ << " pages)\n";
    }

private:
    /**
     * @brief Allocate new page and add objects to free list
     *
     */
    void AllocateNewPage() {
        // page limit
        if (maxPages_ > 0 && currentPages_ >= maxPages_) {
            std::cerr << "[DynamicPool] Maximum pages (" << maxPages_ << ") reached. Cannot allocate more.\n";
            return;
        }

        std::cout << "[DynamicPool] Allocating new page "<< (currentPages_ + 1);
        if (maxPages_ > 0) {
            std::cout << "/" << maxPages_;
        }
      
        // Allocate new page
        Page* newPage = nullptr;
        try {
            newPage = new Page();
        }
        catch (const std::bad_alloc&) {
            std::cerr << "[DynamicPool] Failed to allocate page\n";
            return;
        }

        // Link page to page list
        newPage->next = pageList_;
        pageList_ = newPage;

        // Add all objects from this page to free list
        uint8_t* pageMemory = newPage->memory;
        for (size_t i = 0; i < ObjectsPerPage; ++i) {
            MemoryBlock* block = reinterpret_cast<MemoryBlock*>(pageMemory + (i * sizeof(T)));

            // Push to free list
            block->next = freeList_;
            freeList_ = block;
        }

        // Update
        ++currentPages_;
        totalCapacity_ += ObjectsPerPage;
    }

};

#endif // MEMORYPOOL_H