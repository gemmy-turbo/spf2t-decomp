// SPDX-License-Identifier: GPL-3.0-only
// USA 960620, 0x011974..0x011A19: original pair placement gameplay effects.
#pragma once
#include "pair_cell.hpp"
#include <cstdint>

namespace puzzle_fighter {
struct PairPlacementResult {
    std::uint32_t primary{}, secondary{};
    std::uint16_t rotation_index{}, displacement{}, selected_below{}, metadata_probe{};
    bool first_diamond{}, second_diamond{}, nonzero_below{}, metadata_copied{};
};

// Memory supplies big-endian read_word/read_byte and write_word/write_byte.
// Original table words at 0x012528/0x012530 remain external. Preserve ordered
// accesses and rereads: board writes may alias each other or player fields.
// Addresses wrap at 32 bits here; the memory adapter defines physical mapping.
// Word alignment/access faults and CPU ABI/stack/cycles are outside this API.
template<class Memory>
PairPlacementResult place_pair(std::uint32_t player, Memory &memory) {
    const auto long_word = [&](std::uint32_t address) {
        const auto high = memory.read_word(address);
        const auto low = memory.read_word(address + 2u);
        return (std::uint32_t(high) << 16u) | low;
    };
    PairPlacementResult result;
    memory.write_word(player + 0x1f2u, 0);
    PairCellState cell;
    cell.a[0] = long_word(player + 0x164u) + 2u;
    const auto row = memory.read_word(player + 0x14u);
    const auto column = memory.read_word(player + 0x10u);
    result.primary = pair_cell(cell, row, column).a[0];
    const auto first = memory.read_word(player + 0x152u);
    memory.write_word(result.primary, first);
    result.rotation_index = std::uint16_t(2u * memory.read_byte(player + 0x170u));
    const auto first_offset = memory.read_word(0x12528u + result.rotation_index);
    const auto second_offset = memory.read_word(0x12530u + result.rotation_index);
    result.displacement = std::uint16_t(first_offset + second_offset);
    const auto signed_offset = (result.displacement & 0x8000u)
        ? std::int32_t(result.displacement) - 65536 : std::int32_t(result.displacement);
    result.secondary = result.primary + std::uint32_t(signed_offset);
    const auto second = memory.read_word(player + 0x154u);
    memory.write_word(result.secondary, second);
    result.first_diamond = memory.read_word(player + 0x152u) == 5;
    if (result.first_diamond) {
        memory.write_byte(player + 0x38cu, 1);
        result.selected_below = memory.read_word(result.primary + 0x10u);
        if (result.selected_below == 0 || result.selected_below == 0xff)
            result.selected_below = 5;
    } else if (memory.read_word(player + 0x154u) == 5) {
        result.second_diamond = true;
        memory.write_byte(player + 0x1fau, 1);
        memory.write_byte(player + 0x38cu, 1);
        result.selected_below = memory.read_word(result.secondary + 0x10u);
    }
    result.nonzero_below = result.selected_below != 0;
    if (result.nonzero_below) {
        const auto opponent = long_word(player + 0x160u);
        memory.write_byte(opponent + 0x27fu, 1);
        memory.write_word(player + 0x1f2u, result.selected_below);
        memory.write_byte(player + 0x31fu, 0);
        result.metadata_probe = memory.read_word(result.primary + 0x20u);
        if (result.metadata_probe & 0x80u) {
            result.metadata_copied = true;
            memory.write_byte(player + 0x31fu, 1);
            const auto metadata = memory.read_word(result.primary + 0x420u);
            memory.write_word(player + 0x372u, metadata);
        }
    }
    return result;
}
} // namespace puzzle_fighter
