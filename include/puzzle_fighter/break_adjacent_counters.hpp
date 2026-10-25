// SPDX-License-Identifier: GPL-3.0-only
// USA 960620, 00D6D6..00D76D. Ordered gameplay effects; no CPU interpreter.
#pragma once
#include <cstdint>

namespace puzzle_fighter {
struct AdjacentCountersResult {
    bool disabled{};
    std::uint8_t changed_directions{}; // right=1, left=2, below=4, above=8
};

// Memory supplies read_byte/write_byte and its physical address mapping.
// rows_above is the low word of original D0; its comparison is signed.
// Scratch high-byte reads are retained even though the final write is literal.
template<class Memory>
AdjacentCountersResult break_adjacent_counters(std::uint32_t player,
        std::uint32_t cell, std::uint16_t rows_above, Memory &memory) {
    AdjacentCountersResult result;
    result.disabled = memory.read_byte(player + 0x38cu) != 0;
    if (result.disabled) return result;
    (void)memory.read_byte(cell + 1u); // Original overwritten scratch read.
    const auto visit = [&](std::uint32_t neighbor, std::uint8_t direction,
                           bool permitted) {
        const auto low = memory.read_byte(neighbor + 1u);
        if (low != 7 || !permitted) return;
        (void)memory.read_byte(neighbor); // Original high-nibble scratch arithmetic.
        memory.write_byte(neighbor + 1u, 0x37);
        result.changed_directions |= direction;
    };
    visit(cell + 2u, 1, true);
    visit(cell - 2u, 2, true);
    visit(cell + 0x10u, 4, true);
    visit(cell - 0x10u, 8, rows_above > 1u && rows_above < 0x8000u);
    return result;
}
} // namespace puzzle_fighter
