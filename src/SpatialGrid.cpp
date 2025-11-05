/* ========================================================
 * @file        SpatialGrid.cpp
 * @author      Yasmin Tazi | yasmin.t@digipen.edu
 * @date        30 OCT 2025
 *
 * @brief   Implementation of spatial partitioning system
 *
 * RUBRIC: Physics/Collisions - Option 1
 * Broad phase collision spatial partitioning (Implicit grid)
 *
 * Copyright � 2025 DigiPen, All rights reserved.
 * ========================================================
 */

#include "SpatialGrid.h"
#include <algorithm>
#include <unordered_set>
#include <cmath>

SpatialGrid::SpatialGrid(float cellSize)
    : cellSize(cellSize)
{
    // Reserve space for common case
    grid.reserve(256);
}

void SpatialGrid::Clear() {
    // Clear all entities from grid cells
    for (auto& [cell, entities] : grid) {
        entities.clear();
    }
}

std::pair<int, int> SpatialGrid::WorldToCell(float x, float y) const {
    // Convert world coordinates to grid cell coordinates
    // Using floor division to handle negative coordinates correctly
    return {
        static_cast<int>(std::floor(x / cellSize)),
        static_cast<int>(std::floor(y / cellSize))
    };
}

std::vector<std::pair<int, int>> SpatialGrid::GetOverlappingCells(const AABB& aabb) const {
    // Get the cell range that this AABB overlaps
    auto [minCellX, minCellY] = WorldToCell(aabb.min.x, aabb.min.y);
    auto [maxCellX, maxCellY] = WorldToCell(aabb.max.x, aabb.max.y);

    std::vector<std::pair<int, int>> cells;

    // Reserve space for expected cell count
    int cellCountX = maxCellX - minCellX + 1;
    int cellCountY = maxCellY - minCellY + 1;
    cells.reserve(cellCountX * cellCountY);

    // Add all cells that the AABB overlaps
    for (int x = minCellX; x <= maxCellX; ++x) {
        for (int y = minCellY; y <= maxCellY; ++y) {
            cells.emplace_back(x, y);
        }
    }

    return cells;
}

void SpatialGrid::Insert(Entity entity, const AABB& aabb) {
    // Get all cells this entity's AABB overlaps
    auto cells = GetOverlappingCells(aabb);

    // Insert entity into all overlapping cells
    for (const auto& cell : cells) {
        grid[cell].push_back(entity);
    }
}

std::vector<Entity> SpatialGrid::Query(const AABB& aabb) const {
    std::vector<Entity> results;
    std::unordered_set<Entity> uniqueEntities; // Avoid duplicates

    // Get cells to search (query AABB + 1 cell border for safety)
    auto [minCellX, minCellY] = WorldToCell(aabb.min.x - cellSize, aabb.min.y - cellSize);
    auto [maxCellX, maxCellY] = WorldToCell(aabb.max.x + cellSize, aabb.max.y + cellSize);

    // Search all nearby cells
    for (int x = minCellX; x <= maxCellX; ++x) {
        for (int y = minCellY; y <= maxCellY; ++y) {
            auto it = grid.find({ x, y });
            if (it != grid.end()) {
                // Add all entities in this cell
                for (Entity entity : it->second) {
                    if (uniqueEntities.insert(entity).second) {
                        results.push_back(entity);
                    }
                }
            }
        }
    }

    return results;
}

std::vector<std::pair<Entity, Entity>> SpatialGrid::GetAllPotentialPairs() const {
    std::vector<std::pair<Entity, Entity>> pairs;

    // Set to track pairs we've already added (avoid duplicates)
    std::unordered_set<uint64_t> seenPairs;

    // Helper to create unique pair ID
    auto makePairID = [](Entity a, Entity b) -> uint64_t {
        if (a > b) std::swap(a, b); // Ensure consistent ordering
        return (static_cast<uint64_t>(a) << 32) | static_cast<uint64_t>(b);
        };

    // For each cell in the grid
    for (const auto& [cell, entities] : grid) {
        const auto [cellX, cellY] = cell;

        // Check entities within same cell
        for (size_t i = 0; i < entities.size(); ++i) {
            for (size_t j = i + 1; j < entities.size(); ++j) {
                uint64_t pairID = makePairID(entities[i], entities[j]);
                if (seenPairs.insert(pairID).second) {
                    pairs.emplace_back(entities[i], entities[j]);
                }
            }
        }

        // Check entities in adjacent cells (right, down, down-right, down-left)
        // We only check 4 directions to avoid duplicate checks
        std::vector<std::pair<int, int>> adjacentCells = {
            {cellX + 1, cellY},     // Right
            {cellX, cellY - 1},     // Down
            {cellX + 1, cellY - 1}, // Down-right
            {cellX - 1, cellY - 1}  // Down-left
        };

        for (const auto& adjCell : adjacentCells) {
            auto it = grid.find(adjCell);
            if (it != grid.end()) {
                const auto& adjEntities = it->second;

                // Check all pairs between current cell and adjacent cell
                for (Entity entityA : entities) {
                    for (Entity entityB : adjEntities) {
                        uint64_t pairID = makePairID(entityA, entityB);
                        if (seenPairs.insert(pairID).second) {
                            pairs.emplace_back(entityA, entityB);
                        }
                    }
                }
            }
        }
    }

    return pairs;
}

SpatialGrid::Stats SpatialGrid::GetStats() const {
    Stats stats;
    stats.totalCells = grid.size();
    stats.occupiedCells = 0;
    stats.totalEntities = 0;
    stats.maxEntitiesInCell = 0;

    std::unordered_set<Entity> uniqueEntities;

    for (const auto& [cell, entities] : grid) {
        if (!entities.empty()) {
            stats.occupiedCells++;
            stats.maxEntitiesInCell = std::max(stats.maxEntitiesInCell, entities.size());

            for (Entity e : entities) {
                uniqueEntities.insert(e);
            }
        }
    }

    stats.totalEntities = uniqueEntities.size();
    stats.avgEntitiesPerCell = stats.occupiedCells > 0
        ? static_cast<float>(stats.totalEntities) / stats.occupiedCells
        : 0.0f;

    return stats;
}

SpatialGrid::DebugInfo SpatialGrid::GetDebugInfo() const {
    DebugInfo info;

    for (const auto& [cell, entities] : grid) {
        if (!entities.empty()) {
            const auto [cellX, cellY] = cell;

            // Calculate cell bounds
            AABB cellBounds;
            cellBounds.min = { cellX * cellSize, cellY * cellSize };
            cellBounds.max = { (cellX + 1) * cellSize, (cellY + 1) * cellSize };
            cellBounds.dim = { cellSize, cellSize };

            info.cellBounds.push_back(cellBounds);
            info.cellEntityCounts.push_back(static_cast<int>(entities.size()));
            info.cellCoords.push_back({cellX, cellY});  // Store actual cell coordinates
        }
    }

    return info;
}

void SpatialGrid::MarkCollisionBetween(const AABB& aabbA, const AABB& aabbB) {
    // Calculate the intersection region of the two AABBs
    AABB intersection;
    intersection.min.x = std::max(aabbA.min.x, aabbB.min.x);
    intersection.min.y = std::max(aabbA.min.y, aabbB.min.y);
    intersection.max.x = std::min(aabbA.max.x, aabbB.max.x);
    intersection.max.y = std::min(aabbA.max.y, aabbB.max.y);

    // If there's no valid intersection, don't mark anything
    if (intersection.min.x >= intersection.max.x || intersection.min.y >= intersection.max.y) {
        return;
    }

    // Mark only the cells that overlap with the intersection region
    auto cells = GetOverlappingCells(intersection);
    for (const auto& cell : cells) {
        collisionCells.insert(cell);
    }
}

bool SpatialGrid::CellHasCollision(int cellX, int cellY) const {
    return collisionCells.count({cellX, cellY}) > 0;
}

size_t SpatialGrid::GetCollisionCellCount() const {
    return collisionCells.size();
}

void SpatialGrid::ClearCollisionMarkers() {
    collisionCells.clear();
}