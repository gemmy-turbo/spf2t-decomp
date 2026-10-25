// SPDX-License-Identifier: GPL-3.0-only
// Recovered from verified USA 960620 opcodes 00ECAA..00ECEB.
#pragma once
#include <cstdint>

namespace puzzle_fighter {
struct TidyFieldResult {
    std::uint32_t field = 0;
    std::uint16_t empty_cells = 0, counter_cells = 0;
    std::uint8_t last_compared_byte = 0;
};

// Memory supplies ordered big-endian read_word/read_byte/write_word operations.
// Its bus masks addresses to 24 bits; pointer arithmetic remains 32-bit.
// Re-read after clearing: player/field aliases must see earlier writes.
template<class Memory>
TidyFieldResult tidy_field(std::uint32_t player, Memory &memory) {
    TidyFieldResult result;
    const auto high = memory.read_word(player + 0x164u);
    const auto low = memory.read_word(player + 0x166u);
    result.field = (std::uint32_t(high) << 16) | low;
    for (unsigned column = 0; column != 6; ++column) {
        auto cell = result.field + 0xc2u + 2u * column;
        for (unsigned row = 0; row != 13; ++row, cell -= 0x10u) {
            if (memory.read_byte(cell + 1u) == 0) {
                memory.write_word(cell, 0);
                memory.write_word(cell + 0x400u, 0);
                ++result.empty_cells;
            }
            result.last_compared_byte = memory.read_byte(cell + 1u);
            if (result.last_compared_byte == 7) {
                memory.write_word(cell + 0x400u, 0);
                ++result.counter_cells;
            }
        }
    }
    return result;
}
} // namespace puzzle_fighter
