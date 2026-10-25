// SPDX-License-Identifier: GPL-3.0-only
// USA 960620, 0x00D01C..0x00D057: locate the first diamond in scan order.
#pragma once
#include <cstdint>

namespace puzzle_fighter {
struct DiamondScanResult {
    bool found = false;
    std::uint32_t cell = 0;
    std::uint16_t under = 0, visited = 0;
    std::uint8_t column = 0, row = 0;
};

// Memory provides big-endian read_word/write_word and the 24-bit bus boundary.
// Pointer arithmetic retains 32 bits. A no-match call performs no writes.
template<class Memory>
DiamondScanResult diamond_scan(std::uint32_t player, Memory &memory) {
    const auto high = memory.read_word(player + 0x164u);
    const auto low = memory.read_word(player + 0x166u);
    const auto field = (std::uint32_t(high) << 16) | low;
    DiamondScanResult result;
    for (unsigned column = 0; column < 6; ++column) {
        auto cell = field + 0xc2u + column * 2u;
        for (unsigned row = 0; row < 15; ++row, cell -= 0x10u) {
            ++result.visited;
            if (memory.read_word(cell) != 5) continue;
            const auto below = memory.read_word(cell + 0x10u);
            result.found = true;
            result.cell = cell;
            result.under = below == 0xffu ? 5u : below;
            result.column = static_cast<std::uint8_t>(column);
            result.row = static_cast<std::uint8_t>(row);
            memory.write_word(player + 0x1f2u, result.under);
            return result;
        }
    }
    return result;
}
} // namespace puzzle_fighter
