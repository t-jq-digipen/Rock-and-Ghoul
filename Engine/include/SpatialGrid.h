/* ========================================================
 * @file        SpatialGrid.h
 * @author      Yasmin Tazi | yasmin.t@digipen.edu
 * @date        30 OCT 2025
 *
 * @brief   Spatial partitioning system for collision optimization
 *
 * Implements a uniform grid spatial partitioning structure to
 * reduce collision checks from O(n^2) to O(n) by only checking
 * entities in nearby grid cells.
 *
 *
 * Copyright © 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "Entity.h"
#include "Collision.h"
#include "Remath.h"

 /**
  * @brief Spatial grid for broad-phase collision detection
  *
  * THEORY:
  * Instead of checking every entity against every other entity (O(n^2)),
  * we divide the world into a grid of cells. Entities are placed into
  * cells based on their position. We only check collisions between
  * entities in the same cell or adjacent cells.
  *
  * COMPLEXITY:
  * - Without grid: 100 entities = 100*99/2 = 4,950 checks
  * - With grid:    100 entities = 10 per cell * 9 cells = 90 checks average
  *
  * USAGE:
  * 1. Clear() at start of frame
  * 2. Insert() all entities with physics
  * 3. Query() to get potential collision pairs
  * 4. Do narrow-phase collision on pairs only
  */
class SpatialGrid {
public:
    /**
     * @brief Construct a spatial grid
     * @param cellSize Size of each grid cell (larger = fewer cells, more entities per cell)
     *
     * CHOOSING CELL SIZE:
     * - Too small: Entities span multiple cells, more overhead
     * - Too large: Many entities per cell, less benefit
     * - Good rule: cellSize around 2-3 average entity size
     */
    SpatialGrid(float cellSize = 200.0f);

    /**
     * @brief Clear all entities from grid (call at start of frame)
     */
    void Clear();

    /**
     * @brief Insert an entity into the grid based on its AABB
     * @param entity Entity ID
     * @param aabb Entity's axis-aligned bounding box
     *
     * Entities that span multiple cells are inserted into all overlapping cells.
     */
    void Insert(Entity entity, const AABB& aabb);

    /**
     * @brief Query potential collision pairs near an entity
     * @param aabb Query region (usually entity's AABB)
     * @return List of entities that might collide with query region
     *
     * Returns entities in same cell and adjacent cells (3*3 grid = 9 cells).
     * These are POTENTIAL collisions - still need narrow-phase check!
     */
    std::vector<Entity> Query(const AABB& aabb) const;

    /**
     * @brief Get all unique collision pairs from the entire grid
     * @return Vector of entity pairs that are in nearby cells
     *
     * This generates all potential collision pairs by checking entities
     * in each cell against entities in the same and adjacent cells.
     * Eliminates duplicate pairs automatically.
     */
    std::vector<std::pair<Entity, Entity>> GetAllPotentialPairs() const;

    /**
     * @brief Get statistics for debugging/profiling
     */
    struct Stats {
        size_t totalCells;
        size_t occupiedCells;
        size_t totalEntities;
        float avgEntitiesPerCell;
        size_t maxEntitiesInCell;
    };
    Stats GetStats() const;

    /**
     * @brief Debug visualization data
     */
    struct DebugInfo {
        std::vector<AABB> cellBounds;
        std::vector<int> cellEntityCounts;
        std::vector<std::pair<int, int>> cellCoords;  // Actual cell coordinates
    };
    DebugInfo GetDebugInfo() const;

    /**
     * @brief Mark cells where two AABBs intersect as having a collision
     * @param aabbA First entity's AABB
     * @param aabbB Second entity's AABB
     */
    void MarkCollisionBetween(const AABB& aabbA, const AABB& aabbB);

    /**
     * @brief Check if a cell has a collision this frame
     * @param cellX Cell X coordinate
     * @param cellY Cell Y coordinate
     */
    bool CellHasCollision(int cellX, int cellY) const;

    /**
     * @brief Get count of cells with collisions
     */
    size_t GetCollisionCellCount() const;

    /**
     * @brief Clear collision markers (call at start of frame)
     */
    void ClearCollisionMarkers();

    /**
     * @brief Get cell size for debug rendering
     */
    float GetCellSize() const { return cellSize; }

    // Hash function for grid coordinates (must be public for collision set)
    struct CellHash {
        std::size_t operator()(const std::pair<int, int>& cell) const {
            // Cantor pairing function for unique hash
            return (cell.first + cell.second) * (cell.first + cell.second + 1) / 2 + cell.second;
        }
    };

private:
    float cellSize;

    // Grid structure: map of (x,y) cell coordinates to list of entities
    std::unordered_map<std::pair<int, int>, std::vector<Entity>, CellHash> grid;

    // Set of cells where collisions occurred this frame
    std::unordered_set<std::pair<int, int>, CellHash> collisionCells;

    /**
     * @brief Convert world position to grid cell coordinates
     */
    std::pair<int, int> WorldToCell(float x, float y) const;

    /**
     * @brief Get all cells that an AABB overlaps
     */
    std::vector<std::pair<int, int>> GetOverlappingCells(const AABB& aabb) const;
};